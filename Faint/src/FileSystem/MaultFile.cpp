#include "MaultFile.h"
#include <Debug/Log.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

Moon::Tools::MaultFile::MaultFile(const std::string& pakPath)
	: m_pakPath(pakPath)
{
}

Moon::Tools::MaultFile::~MaultFile() {
	if (m_stream.is_open())
		m_stream.close();
}

bool Moon::Tools::MaultFile::Mount() {
	m_stream.open(m_pakPath, std::ios::binary);
    if (!m_stream.is_open()) {
		HZ_CORE_ERROR("[MaultFile] Failed to open: {}", m_pakPath);
        return false;
    }

	uint32_t fileCount = 0;
	m_stream.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
    if (m_stream.fail()) {
        HZ_CORE_ERROR("[MaultFile] Failed to read fileCount from: {}", m_pakPath);
        return false;
    }

    m_entries.clear();

	for (uint32_t i = 0; i < fileCount; i++) {
		uint16_t nameLen = 0;
		m_stream.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        if (m_stream.fail()) {
            HZ_CORE_ERROR("[MaultFile] Failed to read nameLen at entry: {}", m_pakPath);
            return false;
        }

		std::string name(nameLen, '\0');
		m_stream.read(name.data(), nameLen);
        if (m_stream.fail()) {
            HZ_CORE_ERROR("[MaultFile] Failed to read name at entry: {}", m_pakPath);
            return false;
        }

		VaultEntry entry;
		entry.name = std::move(name);
		m_stream.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
		m_stream.read(reinterpret_cast<char*>(&entry.compressedSize), sizeof(entry.compressedSize));
		m_stream.read(reinterpret_cast<char*>(&entry.unCompressedSize), sizeof(entry.unCompressedSize));
	
        if (m_stream.fail()) {
            HZ_CORE_ERROR("[MaultFile] Failed to read entry metadata at entry: {}", m_pakPath);
            return false;
        }

		m_entries[name] = std::move(entry);
	}

    HZ_CORE_TRACE("[MaultFile] Successfully mounted: {} ({} files)", m_pakPath, m_entries.size());
	return true;
}

bool Moon::Tools::MaultFile::FileExists(const std::string& virtualPath) const {
	return m_entries.find(virtualPath) != m_entries.end();
}

std::vector<uint8_t> Moon::Tools::MaultFile::ReadFile(const std::string& virtualPath) {
    {
        auto it = m_cache.find(virtualPath);
        if (it != m_cache.end()) {
            return it->second;
        }
    }

    auto it = m_entries.find(virtualPath);
    if (it == m_entries.end()) return {};

    const VaultEntry& e = it->second;
    std::vector<uint8_t> compressed(e.compressedSize);
    m_stream.seekg(e.offset);
    m_stream.read(reinterpret_cast<char*>(compressed.data()), e.compressedSize);

    std::vector<uint8_t> out(e.unCompressedSize);
    size_t decompressed = ZSTD_decompress(out.data(), e.unCompressedSize, compressed.data(), e.compressedSize);

    if (ZSTD_isError(decompressed)) {
        std::cerr << "[MaultFile] ZSTD decompress failed: " << ZSTD_getErrorName(decompressed) << "\n";
        return {};
    }

    out.resize(decompressed);

    if (m_cache.size() >= CACHE_LIMIT) {
        auto first = m_cache.begin();
        m_cache.erase(first);
    }
    m_cache[virtualPath] = out;

    return out;
}
