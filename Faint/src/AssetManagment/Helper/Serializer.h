#pragma once

#include <optional>
#include <string>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Moon::Helper {

	using Value = std::variant<int, float, std::string>;

	struct Node {
		enum class Type { Null, Int, Float, String, Array, Object } type;

		using Array = std::vector<Node>;
		using Object = std::unordered_map<std::string, Node>;

		std::variant<std::monostate, int, float, std::string, Array, Object> data;

		Node() : type(Type::Null), data(std::monostate{}) {}
	};

	enum class TokenType {
		Identifier,
		Number,
		String,
		Equal,
		LBrace,
		RBrace,
		LBracket,
		RBracket,
		Semicolon,
		Comma,
		EndOfFile,
		Unknown
	};

	struct Token {
		TokenType type;
		std::string text;
	};

	class Tokenizer {
		std::string input;
		size_t pos = 0;

	public:
		explicit Tokenizer(const std::string& str) : input(str) {}

		char peek() {
			if (pos >= input.size()) return '\0';
			return input[pos];
		}

		char get() {
			if (pos >= input.size()) return '\0';
			return input[pos++];
		}

		void skipWhitespace() {
			while (std::isspace(peek())) get();
		}

		Token nextToken() {
			skipWhitespace();

			char c = peek();
			if (c == '\0') return { TokenType::EndOfFile, "" };

			// Identifiers: a-zA-z0-9_ (start with a-zA-Z or _)
			if (std::isalpha(c) || c == '_') {
				std::string id;
				while (std::isalnum(peek()) || peek() == '_') id += get();
				return { TokenType::Identifier, id };
			}

			// Numbers (int or float)
			if (std::isdigit(c) || c == '-' || c == '+') {
				std::string num;
				bool dotSeen = false;
				if (c == '-' || c == '+') num += get();
				while (std::isdigit(peek()) || peek() == '.') {
					if (peek() == '.') {
						if (dotSeen) break;
						dotSeen = true;
					}
					num += get();
				}
				return { TokenType::Number, num };
			}

			// String literal "..."
			if (c == '"') {
				get(); // skip "
				std::string str;
				while (peek() != '"' && peek() != '\0') {
					if (peek() == '\\') {
						get(); // skip
						char ecs = get();
						if (ecs == 'n') str += '\n';
						else if (ecs == 't') str += '\t';
						else str += ecs;
					}
					else {
						str += get();
					}
				}
				if (peek() == '"') get(); // skip closing "
				return { TokenType::String, str };
			}
			
			// Single character tokens
			switch (c) {
				case '=': get(); return { TokenType::Equal, "=" };
				case '{': get(); return { TokenType::LBrace, "{" };
				case '}': get(); return { TokenType::RBrace, "}" };
				case '[': get(); return { TokenType::LBracket, "[" };
				case ']': get(); return { TokenType::RBracket, "]" };
				case ';': get(); return { TokenType::Semicolon, ";" };
				case ',': get(); return { TokenType::Comma, "m" };
			}

			// Unknown char
			get();
			return { TokenType::Unknown, std::string(1, c) };
		}
	};

	class Parser {
		Tokenizer tokenizer;
		Token currentToken;

		void advance() {
			currentToken = tokenizer.nextToken();
		}

		bool expect(TokenType type) {
			if (currentToken.type == type) {
				advance();
				return true;
			}
			return false;
		}

		//Node parseValue();
		//Node parseObject();
		//Node parseArray();

	public:
		Parser(const std::string& input) : tokenizer(input) {
			advance();
		}

		//Node parse() {
		//	parseObject();
		//}

	private:
		//Node parseValue() {
		//	return Node();
		//}
		//
		//Node parseObject() {
		//	return Node();
		//}
		//
		//Node parseArray() {
		//	return Node();
		//}
	};

	//class Serializer {
	//public:
	//	bool Load(const std::string& filepath);
	//	bool Save(const std::string& p_filepath) const;
	//
	//	/* ======================== */
	//	bool LoadBinary(const std::string& p_filepath);
	//	void SaveBinary(const std::string& p_filepath) const;
	//
	//	std::optional<Value> GetValue(const std::string& key) const;
	//
	//	template<typename T>
	//	std::optional<T> GetAs(const std::string& key) const {
	//		auto val = GetValue(key);
	//		if (val.has_value() && std::holds_alternative<T>(*val))
	//			return std::get<T>(*val);
	//		return std::nullopt;
	//	}
	//
	//	std::unordered_map<std::string, std::string> ParseBlock(const std::string& blockContent);
	//
	//	std::vector<std::unordered_map<std::string, std::string>> ParseBlockArray(const std::string& blockStr);
	//
	//	std::vector<float> ParseFloatArray(const std::string& data);
	//
	//	template<typename T>
	//	std::vector<T> ParseTypedArray(const std::string& data) {
	//		std::vector<T> result;
	//		std::stringstream ss(data);
	//		std::string item;
	//
	//		while (std::getline(ss, item, ';')) {
	//			Trim(item);
	//			try {
	//				if constexpr (std::is_same_v<T, int>) {
	//					result.push_back(std::stoi(item));
	//				}
	//				else if constexpr (std::is_same_v<T, float>) {
	//					result.push_back(std::stof(item));
	//				}
	//			}
	//			catch (...) {
	//				// Skip invalid
	//			}
	//		}
	//		return result;
	//	}
	//
	//	bool ConvertOBJToCBA(const std::string& objPath, const std::string& cbaPath);
	//	/* ================== */
	//
	//	void PrintValues() const;
	//
	//public:
	//	template<typename T>
	//	void WriteArray(std::ofstream& out, const std::vector<T>& data) {
	//		for (size_t i = 0; i < data.size(); i++) {
	//			out << data[i];
	//			if (i != data.size() - 1) out << ';';
	//		}
	//	}
	//private:
	//	/* ===================== */
	//	std::unordered_map<std::string, Value> values;
	//
	//	struct Section {
	//		std::string id;
	//		std::vector<float> vertices;
	//		std::vector<int> indices;
	//	};
	//
	//	static void Trim(std::string& str) {
	//		const char* ws = " '\t\r\n";
	//		str.erase(0, str.find_first_not_of(ws));
	//		str.erase(str.find_last_not_of(ws) + 1);
	//	}
	//	/* ======================== */
	//};
}