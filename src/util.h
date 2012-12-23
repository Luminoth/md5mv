#if !defined __UTIL_H__
#define __UTIL_H__

#if defined USE_OPENSSL
    #include <openssl/md5.h>
#endif

bool is_little_endian();

// returns the error created by using strerror() and errno.
std::string last_std_error() throw();
std::string last_std_error(int error) throw();

// in Windows, returns the error created
// by using FormatMessage() and GetLastError()
// everywhere else, returns last_std_error()
std::string last_error() throw();
std::string last_error(int error) throw();

// returns the time in seconds
double get_time();

#if defined WITH_CRYPTO
// base64 encoding (caller is responsible for freeing the return value)
char* base64_encode(const unsigned char* input, size_t len);
unsigned char* base64_decode(const char* input, size_t& len);

// md5 sum (output must be at least MD5_DIGEST_LENGTH bytes)
void md5sum(const unsigned char* input, size_t len, unsigned char* output);

// md5 sum, base64 encoded (output must be at least (MD5_DIGEST_LENGTH * 2) + 1 bytes)
// this will null terminate the output
void md5sum_hex(const unsigned char* input, size_t len, char* output);

// md5 sum of a file, base64 encoded (same rules apply here as md5sum_hex)
bool md5sum_file(const boost::filesystem::path& path, char* output);

// blowfish encryption (key length is assumed to be 16 bytes, output should have enough space for ilen + 8 bytes)
bool blowfish_encrypt(const unsigned char* key, const unsigned char* input, size_t ilen, unsigned char* output, size_t& olen);
bool blowfish_decrypt(const unsigned char* key, const unsigned char* input, size_t ilen, unsigned char* output, size_t& olen);

std::string md5_digest_password(const std::string& username, const std::string& realm, const std::string& password);
#endif

// creates a hex dump of the given buffer, up to maxlen
std::string bin2hex(const unsigned char* bytes, size_t len, size_t maxlen=1024);

// turns the current process into a daemon
// exits the process on error
void daemonize(bool changedir=true);

// returns true if value is some sort of true boolean value
inline bool to_boolean(const std::string& value)
{
    return strcasecmp(value.c_str(), "true") == 0
        || strcasecmp(value.c_str(), "yes") == 0
        || strcasecmp(value.c_str(), "on") == 0
        || strcasecmp(value.c_str(), "y") == 0
        || value == "1";
}

// returns true if str is an integer value
inline bool is_int(const std::string& str, int base=0)
{
    char* endptr;
    std::strtol(str.c_str(), &endptr, base);
    return (*endptr) == '\0';
}

// returns true if str is a double value
inline bool is_double(const std::string& str)
{
    char* endptr;
    std::strtod(str.c_str(), &endptr);
    return (*endptr) == '\0';
}

/*
 *  A lot of the following were taken from http://graphics.stanford.edu/~seander/bithacks.html
 */

// sets bit i of b
#define SET_BIT(b, i) ((b) |= (0x01 << (i)))

// determines if bit i of b is set
#define GET_BIT(b, i) (((b) >> (i)) & 0x01)

// determines if i is a power of 2
#define POWER_OF_2(i) (!(i & (i - 1)) && i)

// countes the number of bits
// set in i, the Kernighan way
inline unsigned int count_set_bits(unsigned int i)
{
    register unsigned int c;
    for(c = 0; i; ++c) {
        i &= i - 1;
    }
    return c;
}

// determines the parity of i
// true = odd, false = even
inline bool parity(unsigned int i)
{
    bool parity = false;
    while(i) {
        parity = !parity;
        i = i & (i - 1);
    }
    return parity;
}

// swaps n consecutive bits
// starting at i with the n
// consecutive bits starting
// at j in b
inline unsigned int swap_bits(unsigned int b, unsigned int i, unsigned int j, unsigned int n)
{
    int x = ((b >> i) ^ (b >> j)) & ((1 << n) - 1); // XOR temporary
    return b ^ ((x << i) | (x << j));
}

// reverses the bits of i
inline unsigned int reverse_bits(unsigned int b)
{
    register unsigned int t = b;
    for(register size_t i = (sizeof(b) << 3) - 1; i; --i) {
        t |= b & 1;
        t <<= 1;
        b >>= 1;
    }
    return t | (b & 1);
}

#if defined __MATHUTIL_H__
// finds the first set bit
inline unsigned int find_first_set(unsigned int i)
{
    return ilog2(i & -i);
}
#endif

// converts a value
// to a string of bits
inline void value_to_bits(const unsigned int value, char* bits, size_t len)
{
    char* current = bits;
    size_t i=0;
    while(i < len) {
        *current = GET_BIT(value, i) ? '1' : '0';
        current++;
        i++;
    }
}

// rotates x left n times
inline unsigned int rotate_left(unsigned int x, unsigned int n)
{
    return ((x << n) | (x >> (32 - n)));
}

// rotates x right n times
inline unsigned int rotate_right(unsigned int x, unsigned int n)
{
    return ((x >> n) | (x << (32 - n)));
}

inline unsigned char hex_to_byte(unsigned char hex)
{
    return std::isalpha(hex) ? (std::isupper(hex) ? hex - '7' : hex - 'W') : hex - '0';
}

// combines two nibbles into one word
inline unsigned char make_byte(unsigned char msb, unsigned char lsb)
{
    return (msb << 4) | lsb;
}

// combines two bytes into one word
// NOTE: this isn't portable
inline unsigned short make_word(unsigned char msb, unsigned char lsb)
{
    unsigned short ret(0);
    return (((ret | msb) << 8) | lsb);
}

template <typename T>
inline T between(T low, T v, T high)
{
    return MAX(MIN(v, high), low);
}

#endif
