#include "hzpch.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "AssetManager.h"
#include "tiny_obj_loader.h"
#include "Renderer/Renderer.h"
#include "FileSystem/FileSystem.h"
#include "UI/Font/FontLoader.h"

namespace AssetManager {

    std::map<Faint::UUID, std::string> m_assets;
    std::vector<std::string> g_loadLog;

    std::map<std::string, Ref<FaintUI::Font>> m_Fonts;
}

std::map<Faint::UUID, std::string>& AssetManager::GetAssets() {
    return m_assets;
}

std::vector<std::string>& AssetManager::GetLoadLog() {
    return g_loadLog;
}

bool AssetManager::IsAssetLoaded(const Faint::UUID& uuid) {
    return m_assets.find(uuid) != m_assets.end();
}

Ref<Model> AssetManager::LoadModel(const std::string& path) {

    if (path.empty()) {
        HZ_CORE_WARN("[ AssetManager ] File not Found: " + path + " !");
        return nullptr;
    }
    if (!FileSystem::FileExists(path)) {
        HZ_CORE_WARN("[ AssetManager ] File not Found: " + path + " !");
        return nullptr;
    }

    if (!String::EndsWith(path, ".model")) {
        std::string message = "[ AssetManager ] Asset type mismatch file extension.\nExcepted: ";
        HZ_CORE_WARN(message);
    }

    Ref<Model> model = CreateRef<Model>();
    model->id = Faint::UUID();
    model->Path = path;
    //model->Deserialize(j);
    g_loadLog.push_back(path);

    return model;
}

Ref<Material> AssetManager::LoadMaterial(const std::string& path)
{
    if (path.empty()) {
        HZ_CORE_ERROR("[ AssetManager ] File doesn't exists.!\n" + path);
        return nullptr;
    }
    if (!String::EndsWith(path, ".material")) {
        HZ_CORE_ERROR("[ AssetManager ] Extension file is not standard.!\n" + path);
    }

    std::string content = FileSystem::ReadFile(path, true);
    json j = json::parse(content);

    Ref<Material> material = CreateRef<Material>();
    material->Path = path;
    material->Deserialize(j);
    g_loadLog.push_back(path);
    return material;
}

Ref<FaintUI::Font> AssetManager::LoadFont(const std::string& font)
{
    if (m_Fonts.find(font) == m_Fonts.end()) {
        
        //m_Fonts[font] = FaintUI::FontLoader::Get().LoadFont(font);
        return m_Fonts[font];
    }
    else {
        return m_Fonts[font];
    }
}
    
	/*void AssetManager::LoadModel(Model* model) {

		/*tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
        glm::vec3 modelAabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 modelAabbMax = glm::vec3(-std::numeric_limits<float>::max());

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model->_fullpath.c_str())) {
			HZ_CORE_ERROR("LoadModel() failed to load: '{0}'", model->_fullpath);
			return;
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

		for (const auto& shape : shapes) {

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
            glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());

            for (int i = 0; i < shape.mesh.indices.size(); i++) {
                Vertex vertex = {};
                const auto& index = shape.mesh.indices[i];
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                // Check if `normal_index` is zero or positive. negative = no normal data
                if (index.normal_index >= 0) {
                    vertex.normal.x = attrib.normals[3 * size_t(index.normal_index) + 0];
                    vertex.normal.y = attrib.normals[3 * size_t(index.normal_index) + 1];
                    vertex.normal.z = attrib.normals[3 * size_t(index.normal_index) + 2];
                }
                if (attrib.texcoords.size() && index.texcoord_index != -1) { // should only be 1 or 2, there is some bug here where in debug where there were over 1000 on the sphere lines model...
                    vertex.uv = { attrib.texcoords[2 * index.texcoord_index + 0],	1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                aabbMin.x = std::min(aabbMin.x, vertex.position.x);
                aabbMin.y = std::min(aabbMin.y, vertex.position.y);
                aabbMin.z = std::min(aabbMin.z, vertex.position.z);
                aabbMax.x = std::max(aabbMax.x, vertex.position.x);
                aabbMax.y = std::max(aabbMax.y, vertex.position.y);
                aabbMax.z = std::max(aabbMax.z, vertex.position.z);

                indices.push_back(uniqueVertices[vertex]);
            }

            // Tangents
            for (int i = 0; i < indices.size(); i += 3) {
                Vertex* vert0 = &vertices[indices[i]];
                Vertex* vert1 = &vertices[indices[i + 1]];
                Vertex* vert2 = &vertices[indices[i + 2]];
                glm::vec3 deltaPos1 = vert1->position - vert0->position;
                glm::vec3 deltaPos2 = vert2->position - vert0->position;
                glm::vec2 deltaUV1 = vert1->uv - vert0->uv;
                glm::vec2 deltaUV2 = vert2->uv - vert0->uv;
                float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
                glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
                vert0->tangent = tangent;
                vert1->tangent = tangent;
                vert2->tangent = tangent;
            }

            modelAabbMin = Util::Vec3Min(modelAabbMin, aabbMin);
            modelAabbMax = Util::Vec3Max(modelAabbMax, aabbMax);

			std::lock_guard<std::mutex> lock(_modelsMutex);
            model->AddMeshIndex(AssetManager::CreateMesh(shape.name, vertices, indices));
        }

        // Build the bounding box
        float width = std::abs(modelAabbMax.x - modelAabbMin.x);
        float height = std::abs(modelAabbMax.y - modelAabbMin.y);
        float depth = std::abs(modelAabbMax.z - modelAabbMin.z);
        BoundingBox boundingBox;
        boundingBox.size = glm::vec3(width, height, depth);
        boundingBox.offsetFromModelOrigin = modelAabbMin;
        model->SetBoundingBox(boundingBox);
        model->m_aabbMin = modelAabbMin;
        model->m_aabbMax = modelAabbMax;

        // Done
        model->m_loadedFromDisk = true;
	}
    int AssetManager::CreateMesh(std::string name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {

        Mesh& mesh = m_meshes.emplace_back();
        mesh.baseVertex = _nextVertexInsert;
        mesh.baseIndex = _nextIndexInsert;
        mesh.vertexCount = (uint32_t)vertices.size();
        mesh.indexCount = (uint32_t)indices.size();
        mesh.name = name;

        m_vertices.reserve(m_vertices.size() + vertices.size());
        m_vertices.insert(std::end(m_vertices), std::begin(vertices), std::end(vertices));
        m_indices.reserve(m_indices.size() + indices.size());
        m_indices.insert(std::end(m_indices), std::begin(indices), std::end(indices));

        _nextVertexInsert += mesh.vertexCount;
        _nextIndexInsert += mesh.indexCount;

        return (int)m_meshes.size() - 1;
    }*/
