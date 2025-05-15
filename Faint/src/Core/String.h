#pragma once
#include <string>
#include <vector>

namespace Faint {

	class String {
	public:
		static bool BeginsWith(const std::string& string, const std::string& begin);
		static bool EndsWith(const std::string& string, const std::string_view& begin);
		static bool IsDigit(const char& character);
		static std::string RemoveWhiteSpace(const std::string& string);
		static std::string Sanitize(const std::string& keyword);
		static std::vector<std::string> Split(const std::string& string, char delimiter);
	
		static std::string ToUpper(const std::string& string);
		static std::string ReplaceSlash(const std::string& str);
	};
}