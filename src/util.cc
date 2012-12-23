#include "pch.h"
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#if defined USE_OPENSSL
    #include <openssl/bio.h>
    #include <openssl/buffer.h>
    #include <openssl/evp.h>
#endif
#include "util.h"
#include "math_util.h"
#include "fs_util.h"

/* from http://www.cpp-programming.net/c-tidbits/gettimeofday-function-for-windows/ */
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

_CRTIMP extern int errno;

bool is_little_endian()
{
    unsigned int n = 1;
    unsigned char* b = reinterpret_cast<unsigned char*>(&n);
    return (*b) == 1;
}

std::string last_std_error() throw()
{
    return last_std_error(errno);
}

std::string last_std_error(int error) throw()
{
    return std::strerror(error);
}

std::string last_error() throw()
{
#if defined WIN32
    return last_error(GetLastError());
#else
    return last_std_error();
#endif
}

std::string last_error(int error) throw()
{
#if defined WIN32
    wchar_t buffer[512] = { 0 };
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, 0, buffer, 512, NULL);

    char ba[1024] = { 0 };
    wcstombs(ba, buffer, 1024);
    return std::string(ba);
#else
    return last_std_error(error);
#endif
}

double get_time()
{
    timeval tv;

#if defined WIN32
    /* from http://www.cpp-programming.net/c-tidbits/gettimeofday-function-for-windows/ */
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    unsigned __int64 tmpres = 0;
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    // converting file time to unix epoch
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tmpres /= 10;   // convert into microseconds
    tv.tv_sec = static_cast<long>(tmpres / 1000000UL);
    tv.tv_usec = static_cast<long>(tmpres % 1000000UL);
#else
    gettimeofday(&tv, NULL);
#endif

    return tv.tv_sec + (tv.tv_usec * 1e-6);
}

#if defined WITH_USE_CRYPTO
// TODO: this is a bit arbitrary and unnecessary
#define BASE64_MAX_BUFFER 10240

char* base64_encode(const unsigned char* input, size_t len)
{
    // create the encoder
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // create the bit bucket
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    // write the input
    BIO_write(bio, input, len);
    BIO_flush(bio);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio, &bptr);

    // read the result
    char* ret = new char[bptr->length+1];
    memcpy(ret, bptr->data, bptr->length);
    ret[bptr->length] = 0;

    BIO_free_all(bio);
    return ret;
}

unsigned char* base64_decode(const char* input, size_t& len)
{
    // create the decoder
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // create the bit bucket
    BIO* bio = BIO_new_mem_buf(const_cast<char*>(input), -1);
    bio = BIO_push(b64, bio);

    // read the result
    unsigned char scratch[BASE64_MAX_BUFFER];
    len = BIO_read(bio, scratch, BASE64_MAX_BUFFER);
    if(len <= 0) return NULL;

    unsigned char* ret = new unsigned char[len];
    memcpy(ret, scratch, len);

    BIO_free_all(bio);
    return ret;
}

void md5sum(const unsigned char* input, size_t len, unsigned char* output)
{
    MD5(input, len, output);
}

void md5sum_hex(const unsigned char* input, size_t len, char* output)
{
    // get the md5sum
    unsigned char scratch[MD5_DIGEST_LENGTH];
    md5sum(input, len, scratch);

    // write it out as hex
    std::stringstream str;
    for(int i=0; i<MD5_DIGEST_LENGTH; ++i) {
        str << std::hex << std::setfill('0') << std::setw(2) << (int)scratch[i];
    }

    // copy to the output
    std::string encoded(str.str());
    std::copy(encoded.begin(), encoded.end(), output);
    output[MIN(static_cast<int>(encoded.length()), MD5_DIGEST_LENGTH << 1)] = 0;
}

bool md5sum_file(const boost::filesystem::path& path, char* output)
{
    std::string data;
    if(!file_to_string(path, data)) {
        return false;
    }

    md5sum_hex(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), output);
    return true;
}

bool blowfish_encrypt(const unsigned char* key, const unsigned char* input, size_t ilen, unsigned char* output, size_t& olen)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_bf_ecb(), NULL, key, NULL);

    int len;
    if(!EVP_EncryptUpdate(&ctx, output, &len, input, ilen)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    if(!EVP_EncryptFinal_ex(&ctx, output + len, &len)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    EVP_CIPHER_CTX_cleanup(&ctx);
    return true;
}

bool blowfish_decrypt(const unsigned char* key, const unsigned char* input, size_t ilen, unsigned char* output, size_t& olen)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_DecryptInit_ex(&ctx, EVP_bf_ecb(), NULL, key, NULL);

    int len;
    if(!EVP_DecryptUpdate(&ctx, output, &len, input, ilen)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    if(!EVP_DecryptFinal_ex(&ctx, output + len, &len)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    EVP_CIPHER_CTX_cleanup(&ctx);
    return true;
}

std::string md5_digest_password(const std::string& username, const std::string& realm, const std::string& password)
{
    std::string key(username + ":" + realm + ":" + password);

    char digest[(MD5_DIGEST_LENGTH << 1) + 1];
    md5sum_hex(reinterpret_cast<const unsigned char*>(key.c_str()), key.length(), digest);
    return digest;
}
#endif

std::string bin2hex(const unsigned char* bytes, size_t len, size_t maxlen)
{
    static const size_t cpg = 8;                                // line characters per group
    static const size_t cpr = cpg << 1;                         // line characters per row
    static const size_t columns = ((cpr * 3) + (cpr * 2)) + 4;  // actual characters per row, including spaces and newlines

    len = maxlen > 0 ? MIN(len, maxlen) : len;
    const size_t rows = static_cast<size_t>(std::ceil(static_cast<float>(len) / static_cast<float>(cpr)));
    const size_t size = columns * rows;

    size_t pos=0;
    boost::scoped_array<char> ret(new char[size + 1]);
    for(size_t row=0; row<rows; ++row) {
        // hex
        for(size_t col=0; col<cpr; ++col) {
            std::string str = "  ";

            size_t loc = (row * cpr) + col;
            if(loc < len) {
                std::stringstream hexstr;
                hexstr.width(2);
                hexstr.fill('0');
                hexstr << std::hex << static_cast<int>(bytes[loc]) << std::dec;
                str = hexstr.str();
            }

            ret.get()[pos] = str[0];
            ret.get()[pos+1] = str[1];
            ret.get()[pos+2] = ' ';
            pos += 3;

            if((col+1) % cpg == 0) {
                ret.get()[pos] = ' ';
                pos++;
            }
        }

        // ascii
        for(size_t col=0; col<cpr; ++col) {
            char ch = ' ';

            size_t loc = (row * cpr) + col;
            if(loc < len) {
                ch = static_cast<char>(bytes[loc]);
                if(!isgraph(ch))
                    ch = '.';
            }

            ret.get()[pos] = ch;
            ret.get()[pos+1] = ' ';
            pos += 2;

            if((col+1) % cpg == 0) {
                ret.get()[pos] = ' ';
                pos++;
            }
        }

        ret.get()[pos-1] = '\n';
    }

    ret.get()[size] = '\0';
    return std::string("\n") + ret.get();
}

void daemonize(bool changedir)
{
    std::cout << "Daemonizing..." << std::endl;

#if defined WIN32
/* TODO: write meh */
#else
    // return control to the shell and drop process group lead
    pid_t pid = fork();
    if(pid < 0) {
        std::cerr << "Fork error: " << std::strerror(errno) << std::endl;
        exit(1);
    } else if(pid > 0) {
        exit(0);
    }

    // regain process group lead and drop controlling terminal
    if(setsid() < 0) {
        std::cerr << "Could not create new process group: " << std::strerror(errno) << std::endl;
        exit(1);
    }

    // drop process group lead so init will clean us up
    pid = fork();
    if(pid < 0) {
        std::cerr << "Fork error: " << std::strerror(errno) << std::endl;
        exit(1);
    } else if(pid > 0) {
        exit(0);
    }

    // move to / so we don't block a filesystem
    if(changedir) {
        chdir("/");
    }

    // keep control over what we write (optional)
    umask(0);

    // ignore SIGHUP (optional)
    //signal(SIGHUP, SIG_IGN);

    // close all old file descriptors
    long maxfd = sysconf(_SC_OPEN_MAX);
    for(int i=0; i<maxfd; ++i) {
        close(i);
    }

    // TODO: error handle this (and ensure the right fd is set)
    open("/dev/null", O_RDONLY);    // stdin
    open("/dev/null", O_RDWR);      // stdout
    open("/dev/null", O_RDWR);      // stderr
#endif
}
