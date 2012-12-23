#include "pch.h"
#include <boost/tokenizer.hpp>
#include "string_util.h"

void tokenize(const std::string& str, Tokens& tokens, const std::string& delimiters)
{
    boost::char_separator<char> sep(delimiters.c_str());
    boost::tokenizer<boost::char_separator<char> > tok(str, sep);
    BOOST_FOREACH(const std::string& token, tok) {
        tokens.push_back(token);
    }
}

template<> std::string to_string<bool>(const bool& t)
{
    return t ? "true" : "false";
}

std::string trim_all(const std::string& str)
{
    std::string ret;
    const size_t len = str.length();
    size_t i=0;
    while(i < len) {
        if(!std::isspace(str[i])) {
            ret += str[i];
        }
        i++;
    }
    return ret;
}

std::string trim_sharp_comment(const std::string& str)
{
    size_t pos = str.find_first_of('#');
    if(std::string::npos != pos) {
        return str.substr(0, pos);
    }
    return str;
}

std::string operator+(char* str1, const std::string& str2)
{
    return std::string(str1) + str2;
}
