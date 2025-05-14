#include "Util.h"

namespace Util {

	std::string Lowercase(std::string& str) {
		std::string result = "";
        for (auto& c : str) {
            result += std::tolower(c);
        }
        return result;
	}

	bool StrCmp(const char* queryA, const char* queryB) {
		if (strcmp(queryA, queryB) == 0)
            return true;
        else
            return false;
	}

	const char* CopyConstChar(const char* text) {
		char* b = new char[strlen(text) + 1] {};
		std::copy(text, text + strlen(text), b);
		return b;
	}

	std::string ReadTextFromFile(std::string path) {
		std::ifstream file(path);
		std::string str;
		std::string line;
		while (std::getline(file, line)) {
			str += line + "\n";
		}
		return str;
	}

	std::string FloatToString(float value, int precision) {
		return std::format("{:.{}f}", value, precision);
	}

	std::string BoolToString(bool value) {
		return value ? "True" : "False";
	}
}