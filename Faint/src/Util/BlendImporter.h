#pragma once

#include <Types/Renderer/Model.h>

namespace Moon::BlendData {
	struct BlendHeader {
		char magic[7]; // "BLENDER"
		char ptrSize; // '_' (32bit) or '-' (64bit)
		char edian; // 'v' (little) or 'V' (big)
		char version[3];
	};
	struct BHead {
		char code[4];
		uint32_t len;
		uint64_t old; // pointer (32/64-bit)
		int32_t SDNAnr;
		int32_t nr;
	};
	struct BlendBlock {
		BHead header;
		std::vector<char> data;
	};
	class BlendFile {
	public:
		bool Load(const std::string& filepath);
		std::vector<BlendBlock> blocks;
		BlendHeader header;
	};

	struct FieldInfo {
		std::string name;
		std::string type;
		size_t offset;
	};

	struct StructInfo {
		std::string name;
		size_t size;
		std::vector<FieldInfo> fields;
	};
	class BlendDNA {
	public:
		bool Parse(const BlendFile& blend);
		StructInfo* GetStruct(const std::string& name);
	private:
		std::vector<std::string> names;
		std::vector<std::string> types;
		std::vector<size_t> tlen;
		std::vector<StructInfo> structs;
	};
}

namespace Moon::BlendImporter {
	
	std::vector<MeshData> Extract(const BlendData::BlendFile& blend, BlendData::BlendDNA& dna);
	ModelData ImportBlend(const std::string& filepath);
}