#include "String.h"
#include <iostream>
#include <regex>
#include <sstream>

namespace Faint {

    bool String::BeginsWith(const std::string& string, const std::string& begin) {
        
        if (string.rfind(begin, 0) == 0) {
            return true;
        }

        return false;
    }
    bool String::EndsWith(const std::string& string, const std::string_view& end) {
        
        if (string.length() >= end.length()) {
            bool result = string.ends_with(end);
            return result;
        }

        return false;
    }

    bool String::IsDigit(const char& character) {
        return character <= 57 && character >= 48;
    }

    std::string String::RemoveWhiteSpace(const std::string& string) {
        
        std::string result = string;
        result.erase(std::remove_if(result.begin(), result.end(), isspace), result.end());
        return result;
    }

    std::string String::Sanitize(const std::string& keyword) {
        
        std::string sanitizedKeyword = keyword;

        // Remove spaces, underscores, and hyphens using regex
        sanitizedKeyword = std::regex_replace(sanitizedKeyword, std::regex("[ _-]+"), "");

        return sanitizedKeyword;
    }

    std::vector<std::string> String::Split(const std::string& string, char delimiter)
    {
        std::vector<std::string> result;
        std::stringstream ss(string);
        std::string item;

        while (getline(ss, item, delimiter))
        {
            result.push_back(item);
        }

        return result;
    }

    std::string String::ToUpper(const std::string& string)
    {
        if (string.length() == 0)
        {
            return "";
        }

        auto split = Split(string, ' ');
        std::string result;
        for (auto& word : split)
        {
            word[0] = std::toupper(word[0]);
            result += word;
        }

        return result;
    }

    std::string String::ReplaceSlash(const std::string& str)
    {
        std::string result = str;
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
}
