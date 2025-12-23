#include "BlendImporter.h"
#include <Debug/Log.h>
#include <fstream>

namespace Moon::BlendImporter {
    struct MVert {
        float co[3];
        short no[3];
        char flag;
        char bweight;
    };
    struct MLoop {
        uint32_t v;
        uint32_t e;
    };
    struct MPoly {
        uint32_t loopstart;
        uint32_t totloop;
        uint32_t mat_nr;
        uint32_t flag;
    };
    struct MLoopUV {
        float uv[2];
        float flag; // بعضاً ممکن است موجود باشد
    };
}

std::vector<Moon::MeshData> Moon::BlendImporter::Extract(const BlendData::BlendFile& blend, BlendData::BlendDNA& dna) {
    std::vector<MeshData> meshes;

    for (const auto& block : blend.blocks) {
        if (strncmp(block.header.code, "ME", 2) == 0) { // Mesh
            MeshData mesh;
            mesh.name = "Mesh_" + std::to_string(meshes.size());
            const char* data = block.data.data();

            auto meshStruct = dna.GetStruct("Mesh");
            if (!meshStruct) {
                HZ_CORE_WARN("Mesh structure not found in DNA");
                continue;
            }

            uint32_t totvert = 0, totloop = 0, totpoly = 0;
            uint64_t mvert_ptr = 0, mloop_ptr = 0, mpoly_ptr = 0, mloopuv_ptr = 0;

            for (const auto& field : meshStruct->fields) {
                if (field.name == "totvert") {
                    totvert = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    HZ_CORE_TRACE("totvert: {}", totvert);
                }
                else if (field.name == "totloop") {
                    totloop = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    HZ_CORE_TRACE("totloop: {}", totloop);
                }
                else if (field.name == "totpoly") {
                    totpoly = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    HZ_CORE_TRACE("totpoly: {}", totpoly);
                }
                else if (field.name == "mvert" || field.name == "vert") {
                    // برای فایل‌های 64 بیتی
                    if (blend.header.ptrSize == '-') {
                        mvert_ptr = *reinterpret_cast<const uint64_t*>(data + field.offset);
                    }
                    else {
                        mvert_ptr = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    }
                    HZ_CORE_TRACE("mvert pointer: {}", mvert_ptr);
                }
                else if (field.name == "mloop" || field.name == "loop") {
                    if (blend.header.ptrSize == '-') {
                        mloop_ptr = *reinterpret_cast<const uint64_t*>(data + field.offset);
                    }
                    else {
                        mloop_ptr = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    }
                }
                else if (field.name == "mpoly" || field.name == "poly") {
                    if (blend.header.ptrSize == '-') {
                        mpoly_ptr = *reinterpret_cast<const uint64_t*>(data + field.offset);
                    }
                    else {
                        mpoly_ptr = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    }
                }
                else if (field.name == "mloopuv" || field.name == "mtface" || field.name == "uv") {
                    if (blend.header.ptrSize == '-') {
                        mloopuv_ptr = *reinterpret_cast<const uint64_t*>(data + field.offset);
                    }
                    else {
                        mloopuv_ptr = *reinterpret_cast<const uint32_t*>(data + field.offset);
                    }
                }
            }

            if (totvert == 0 || totloop == 0 || totpoly == 0) {
                HZ_CORE_WARN("Mesh has zero vertices/loops/polys: v={}, l={}, p={}",
                    totvert, totloop, totpoly);
                continue;
            }

            const MVert* mverts = nullptr;
            const MLoop* mloops = nullptr;
            const MPoly* mpolys = nullptr;
            const MLoopUV* mloopuvs = nullptr;

            // جستجو برای داده‌های واقعی در بلوک‌ها
            for (const auto& data_block : blend.blocks) {
                if (data_block.header.old == mvert_ptr) {
                    mverts = reinterpret_cast<const MVert*>(data_block.data.data());
                }
                if (data_block.header.old == mloop_ptr) {
                    mloops = reinterpret_cast<const MLoop*>(data_block.data.data());
                }
                if (data_block.header.old == mpoly_ptr) {
                    mpolys = reinterpret_cast<const MPoly*>(data_block.data.data());
                }
                if (data_block.header.old == mloopuv_ptr) {
                    mloopuvs = reinterpret_cast<const MLoopUV*>(data_block.data.data());
                }
            }

            if (!mverts || !mloops || !mpolys) {
                HZ_CORE_ERROR("Failed to find mesh data blocks");
                continue;
            }

            mesh.vertices.resize(totvert);
            for (uint32_t i = 0; i < totvert; i++) {
                auto& v = mesh.vertices[i];
                v.position = glm::vec3(mverts[i].co[0], mverts[i].co[1], mverts[i].co[2]);

                // تبدیل normal از short به float
                if (mverts[i].no[0] != 0 || mverts[i].no[1] != 0 || mverts[i].no[2] != 0) {
                    mesh.vertices[i].normal = glm::normalize(glm::vec3(
                        float(mverts[i].no[0]) / 32767.0f,
                        float(mverts[i].no[1]) / 32767.0f,
                        float(mverts[i].no[2]) / 32767.0f
                    ));
                }
                else {
                    mesh.vertices[i].normal = glm::vec3(0, 1, 0); // مقدار پیش‌فرض
                }

                mesh.vertices[i].uv = glm::vec2(0, 0); // مقدار پیش‌فرض

                // به‌روزرسانی AABB
                mesh.aabbMin = glm::min(mesh.aabbMin, mesh.vertices[i].position);
                mesh.aabbMax = glm::max(mesh.aabbMax, mesh.vertices[i].position);
            }
            mesh.vertexCount = totvert;

            // ساخت triangles از polygons
            for (uint32_t i = 0; i < totpoly; i++) {
                const MPoly& poly = mpolys[i];
                mesh.materialIndex = poly.mat_nr;

                // triangulation - تبدیل polygon به مثلث‌ها
                for (uint32_t j = 1; j < poly.totloop - 1; j++) {
                    uint32_t idx0 = mloops[poly.loopstart].v;
                    uint32_t idx1 = mloops[poly.loopstart + j].v;
                    uint32_t idx2 = mloops[poly.loopstart + j + 1].v;

                    mesh.indices.push_back(idx0);
                    mesh.indices.push_back(idx1);
                    mesh.indices.push_back(idx2);

                    // اضافه کردن UVها اگر موجود باشند
                    if (mloopuvs) {
                        mesh.vertices[idx0].uv = glm::vec2(
                            mloopuvs[poly.loopstart].uv[0],
                            mloopuvs[poly.loopstart].uv[1]
                        );
                        mesh.vertices[idx1].uv = glm::vec2(
                            mloopuvs[poly.loopstart + j].uv[0],
                            mloopuvs[poly.loopstart + j].uv[1]
                        );
                        mesh.vertices[idx2].uv = glm::vec2(
                            mloopuvs[poly.loopstart + j + 1].uv[0],
                            mloopuvs[poly.loopstart + j + 1].uv[1]
                        );
                    }
                }
            }

            mesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
            HZ_CORE_INFO("Mesh created: {} vertices, {} indices", mesh.vertexCount, mesh.indexCount);

            meshes.push_back(std::move(mesh));
        }
    }
    HZ_CORE_INFO("Total meshes extracted: {}", meshes.size());
    return meshes;
}

Moon::ModelData Moon::BlendImporter::ImportBlend(const std::string& filepath) {
    Moon::ModelData modelData;
    BlendData::BlendFile file;
    if (!file.Load(filepath)) return modelData;

    BlendData::BlendDNA dna;
    dna.Parse(file);
        //return modelData;

    std::vector<std::string> materialNames;
    for (const auto& block : file.blocks) {
        if (strncmp(block.header.code, "MA", 2) == 0) {
            const char* data = block.data.data();
            std::string name = std::string(data);
            materialNames.push_back(name);
        }
    }
    modelData.materialNames = materialNames;

    modelData.meshes = Extract(file, dna);
    HZ_CORE_TRACE("{}", modelData.meshes.size());
    modelData.name = std::filesystem::path(filepath).stem().string();
    modelData.meshCount = static_cast<uint32_t>(modelData.meshes.size());
    modelData.timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    for (const auto& mesh : modelData.meshes) {
        modelData.aabbMin = glm::min(modelData.aabbMin, mesh.aabbMin);
        modelData.aabbMax = glm::max(modelData.aabbMax, mesh.aabbMax);
    }

    return modelData;
}

bool Moon::BlendData::BlendFile::Load(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        HZ_CORE_ERROR("[BlendImporter] Failed to open file: {}", filepath);
        return false;
    }

    BlendHeader header;
    HZ_CORE_INFO("Blender file version: {}.{}.{}",
        header.version[0], header.version[1], header.version[2]);

    // Read and validate the header
    file.read(reinterpret_cast<char*>(&header), sizeof(BlendHeader));
    if (strncmp(header.magic, "BLENDER", 7) != 0) {
        HZ_CORE_ERROR("[BlendImporter] Not a blender file: {}", filepath);
        return false;
    }

    while (!file.eof()) {
        BHead bhead{};
        file.read(reinterpret_cast<char*>(&bhead), sizeof(BHead));
        if (file.eof()) break;

        if (strcmp(bhead.code, "ENDB") == 0) break;

        std::vector<char> data(bhead.len);
        file.read(data.data(), bhead.len);

        blocks.push_back({ bhead, std::move(data) });
    }

    return true;
}

bool Moon::BlendData::BlendDNA::Parse(const BlendFile& blend) {
    HZ_CORE_INFO("Starting DNA parsing...");

    for (const auto& block : blend.blocks) {
        if (strncmp(block.header.code, "DNA1", 4) == 0) {
            HZ_CORE_INFO("Found DNA1 block, size: {}", block.data.size());

            const char* data = block.data.data();
            size_t offset = 0;
            size_t dataSize = block.data.size();

            // بررسی signature اولیه
            std::string signature(data, std::min(size_t(8), dataSize));
            HZ_CORE_INFO("DNA signature: {}", signature);

            // انتظار داریم signature با "SDNANAME" شروع بشه
            if (signature != "SDNANAME") {
                HZ_CORE_ERROR("Invalid DNA signature: {}", signature);
                return false;
            }

            offset += 8; // رد شدن از "SDNANAME"

            // خواندن بخش‌های NAME, TYPE, TLEN, STRC
            while (offset + 4 <= dataSize) {
                std::string section(data + offset, 4);
                offset += 4;

                HZ_CORE_INFO("Processing section: '{}' at offset {}", section, offset - 4);

                if (section == "NAME") {
                    if (offset + 4 > dataSize) {
                        HZ_CORE_ERROR("Insufficient data for name count");
                        return false;
                    }

                    uint32_t nameCount = *reinterpret_cast<const uint32_t*>(data + offset);
                    offset += 4;

                    if (nameCount == 0 || nameCount > 100000) {
                        HZ_CORE_ERROR("Invalid nameCount: {}", nameCount);
                        return false;
                    }

                    HZ_CORE_INFO("Name count: {}", nameCount);
                    names.clear();
                    names.reserve(nameCount);

                    for (uint32_t i = 0; i < nameCount && offset < dataSize; ++i) {
                        size_t len = strnlen(data + offset, dataSize - offset);
                        if (len == dataSize - offset) {
                            HZ_CORE_ERROR("Name not null-terminated at offset {}", offset);
                            return false;
                        }
                        names.emplace_back(data + offset, len);
                        offset += len + 1;
                        HZ_CORE_TRACE("Name[{}]: '{}'", i, names.back());
                    }
                }
                else if (section == "TYPE") {
                    if (offset + 4 > dataSize) {
                        HZ_CORE_ERROR("Insufficient data for type count");
                        return false;
                    }

                    uint32_t typeCount = *reinterpret_cast<const uint32_t*>(data + offset);
                    offset += 4;

                    if (typeCount == 0 || typeCount > 10000) {
                        HZ_CORE_ERROR("Invalid typeCount: {}", typeCount);
                        return false;
                    }

                    HZ_CORE_INFO("Type count: {}", typeCount);
                    types.clear();
                    types.reserve(typeCount);

                    for (uint32_t i = 0; i < typeCount && offset < dataSize; ++i) {
                        size_t len = strnlen(data + offset, dataSize - offset);
                        if (len == dataSize - offset) {
                            HZ_CORE_ERROR("Type not null-terminated at offset {}", offset);
                            return false;
                        }
                        types.emplace_back(data + offset, len);
                        offset += len + 1;
                        HZ_CORE_TRACE("Type[{}]: '{}'", i, types.back());
                    }
                }
                else if (section == "TLEN") {
                    if (types.empty()) {
                        HZ_CORE_ERROR("TLEN section found before TYPE section");
                        return false;
                    }

                    tlen.resize(types.size());
                    for (size_t i = 0; i < types.size() && offset + 2 <= dataSize; ++i) {
                        tlen[i] = *reinterpret_cast<const uint16_t*>(data + offset);
                        offset += 2;
                        HZ_CORE_TRACE("TypeLen[{}]: {} for type '{}'", i, tlen[i], types[i]);
                    }
                }
                else if (section == "STRC") {
                    if (offset + 4 > dataSize) {
                        HZ_CORE_ERROR("Insufficient data for struct count");
                        return false;
                    }

                    uint32_t structCount = *reinterpret_cast<const uint32_t*>(data + offset);
                    offset += 4;

                    if (structCount > 10000) {
                        HZ_CORE_ERROR("Invalid structCount: {}", structCount);
                        return false;
                    }

                    HZ_CORE_INFO("Struct count: {}", structCount);
                    structs.clear();
                    structs.reserve(structCount);

                    for (uint32_t i = 0; i < structCount && offset + 4 <= dataSize; ++i) {
                        uint16_t typeIndex = *reinterpret_cast<const uint16_t*>(data + offset);
                        offset += 2;
                        uint16_t fieldCount = *reinterpret_cast<const uint16_t*>(data + offset);
                        offset += 2;

                        if (typeIndex >= types.size()) {
                            HZ_CORE_ERROR("Invalid type index: {} (max: {})", typeIndex, types.size());
                            return false;
                        }

                        StructInfo s;
                        s.name = types[typeIndex];
                        s.size = 0;

                        size_t fieldOffset = 0;
                        for (uint32_t f = 0; f < fieldCount && offset + 4 <= dataSize; ++f) {
                            uint16_t nameIndex = *reinterpret_cast<const uint16_t*>(data + offset);
                            offset += 2;
                            uint16_t typeIndexField = *reinterpret_cast<const uint16_t*>(data + offset);
                            offset += 2;

                            if (nameIndex >= names.size() || typeIndexField >= types.size()) {
                                HZ_CORE_ERROR("Invalid indices: nameIndex={}, typeIndexField={}", nameIndex, typeIndexField);
                                return false;
                            }

                            FieldInfo field;
                            field.name = names[nameIndex];
                            field.type = types[typeIndexField];
                            field.offset = fieldOffset;

                            auto it = std::find(types.begin(), types.end(), field.type);
                            if (it != types.end()) {
                                size_t idx = std::distance(types.begin(), it);
                                fieldOffset += idx < tlen.size() ? tlen[idx] : 4;
                            }
                            else {
                                fieldOffset += 4;
                            }

                            s.fields.push_back(field);
                            HZ_CORE_TRACE("  Field[{}]: {} {} (offset: {})", f, field.type, field.name, field.offset);
                        }

                        s.size = fieldOffset;
                        structs.push_back(s);
                        HZ_CORE_INFO("Struct[{}]: '{}' with {} fields", i, s.name, fieldCount);
                    }

                    HZ_CORE_INFO("DNA parsing completed successfully");
                    return true;
                }
                else {
                    HZ_CORE_WARN("Unknown section: '{}', skipping", section);
                    // ادامه می‌دهیم تا بخش بعدی
                }
            }

            HZ_CORE_ERROR("Reached end of DNA block without completing");
            return false;
        }
    }

    HZ_CORE_ERROR("No DNA1 block found");
    return false;
}

Moon::BlendData::StructInfo* Moon::BlendData::BlendDNA::GetStruct(const std::string& name) {
    for (auto& s : structs) if (s.name == name) return &s;
    return nullptr;
}
