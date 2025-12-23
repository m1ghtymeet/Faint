#pragma once

#include <string>
#include <unordered_map>

namespace Moon::Tools {

	template<typename T>
	concept SupportedIniType =
		std::same_as<T, bool> ||
		std::same_as<T, std::string> ||
		std::integral<T> ||
		std::floating_point<T>;

	class IniFile final {
	public:
		using AttributePair = std::pair<std::string, std::string>;
		using AttributeMap = std::unordered_map<std::string, std::string>;

		IniFile(const std::string& p_filepath);

		~IniFile();

		void Rewrite() const;

		template<SupportedIniType T>
		T Get(const std::string& p_key) const;

		bool IsKeyExisting(const std::string& p_key) const;

	private:
		void RegisterPair(const std::string& p_key, const std::string& p_value);
		void RegisterPair(const AttributePair& p_pair);

		void Load();

	private:
		std::string m_fullPath;
		AttributeMap m_data;
	};
}