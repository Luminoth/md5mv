#if !defined __STRING_UTIL_H__
#define __STRING_UTIL_H__

typedef std::deque<std::string> Tokens;

// tokenizes a string
void tokenize(const std::string& str, Tokens& tokens, const std::string& delimiters=" ");

template<typename T>
std::string to_string(const T& t)
{
    std::stringstream ostr;
    ostr << t;
    return ostr.str();
}

template<> std::string to_string<bool>(const bool& t);

// trims all whitespace from a string, including internal whitespace, and returns the trimmed string
std::string trim_all(const std::string& str);

// trims the first occurence of '#' and everything after it..
std::string trim_sharp_comment(const std::string& str);

std::string operator+(char* str1, const std::string& str2);

#endif
