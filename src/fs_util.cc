#include "pch.h"
#include <fstream>
#include <iostream>
#include "fs_util.h"

boost::filesystem::path home_dir()
{
    char* dir = NULL;

#if defined WIN32
    dir = "C:\\";

    TCHAR home_dir[MAX_PATH];
    ZeroMemory(home_dir, MAX_PATH * sizeof(TCHAR));

    HANDLE token = 0;
    if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        DWORD size = MAX_PATH;
        if(GetUserProfileDirectory(token, home_dir, &size)) {
            //dir = home_dir;
        }
        CloseHandle(token);
    }
#else
    dir = std::getenv("HOME");
#endif

    return dir;
}

void list_directory(const boost::filesystem::path& path, std::vector<std::string>& files)
{
    if(boost::filesystem::exists(path)) {
        boost::filesystem::directory_iterator eitr;
        for(boost::filesystem::directory_iterator ditr(path); ditr != eitr; ++ditr) {
            if(boost::filesystem::is_regular_file(ditr->status())) {
#if BOOST_VERSION >= 104600
                files.push_back(ditr->path().filename().string());
#else
                files.push_back(ditr->path().filename());
#endif
            }
        }
    }
}

bool file_to_string(const boost::filesystem::path& path, std::string& str)
{
    std::ifstream file(path.string().c_str());
    if(!file.is_open()) {
        std::cerr << "file: " << path << " does NOT exist!" << std::endl;
        return false;
    }

    str.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return true;
}

bool file_to_strings(const boost::filesystem::path& path, std::vector<std::string>& s)
{
    std::ifstream is(path.string().c_str());
    if(!is.is_open()) {
        std::cerr << "file: " << path.string() << " does NOT exist!" << std::endl;
        return false;
    }

    while(!is.eof()) {
        std::string line;
        std::getline(is, line);
        s.push_back(line);
    }
    return true;
}
