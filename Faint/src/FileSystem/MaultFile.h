#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

namespace Moon::Tools {

	struct VaultEntry {
		std::string name;
		uint64_t offset;
		uint64_t compressedSize;
		uint64_t unCompressedSize;
	};

	class MaultFile {
	public:
		MaultFile(const std::string& pakPath);
		~MaultFile();

		bool Mount();
		bool FileExists(const std::string& virtualPath) const;
		std::vector<uint8_t> ReadFile(const std::string& virtualPath);

	private:
		std::string m_pakPath;
		std::ifstream m_stream;
		std::unordered_map<std::string, VaultEntry> m_entries;

		mutable std::unordered_map<std::string, std::vector<uint8_t>> m_cache;
		static constexpr size_t CACHE_LIMIT = 50;
	};
}