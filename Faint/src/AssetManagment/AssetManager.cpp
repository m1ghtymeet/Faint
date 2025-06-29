#include "hzpch.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "AssetManager.h"
#include "tiny_obj_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Core/String.h"
#include "Renderer/Renderer.h"
#include "FileSystem/FileSystem.h"
#include "Util/Util.h"

namespace AssetManager {

    std::map<Faint::UUID, std::string> m_assets;
    std::vector<std::string> g_loadLog;
    std::vector<Ref<Model>> g_models;
    std::vector<Ref<Mesh>> g_meshes;

    std::map<std::string, Ref<Texture>> m_texturesMap;

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
        glm::mat4 to;

        to[0][0] = from.a1; to[0][1] = from.b1;  to[0][2] = from.c1; to[0][3] = from.d1;
        to[1][0] = from.a2; to[1][1] = from.b2;  to[1][2] = from.c2; to[1][3] = from.d2;
        to[2][0] = from.a3; to[2][1] = from.b3;  to[2][2] = from.c3; to[2][3] = from.d3;
        to[3][0] = from.a4; to[3][1] = from.b4;  to[3][2] = from.c4; to[3][3] = from.d4;

        return to;
    }

    std::vector<Vertex> ProcessVertices(aiMesh* mesh) {

        auto vertices = std::vector<Vertex>();
        vertices.reserve(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex{};

            glm::vec3 current;

            // Position
            vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

            // Tangents
            if (mesh->mTangents) {
                vertex.tangent = {
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z
                };
            }
            vertex.uv = glm::vec2(0.0f, 0.0f);

            // Does it contain UVs?
            if (mesh->mTextureCoords[0]) {
                vertex.uv = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            vertices.push_back(std::move(vertex));
        }

        return vertices;
    }

    std::vector<uint32_t> ProcessIndices(aiMesh* mesh) {

        auto indices = std::vector<uint32_t>();
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        return indices;
    }

    Ref<Mesh> ProcessMesh(aiMesh* meshNode, aiNode* node, const aiScene* scene) {
        std::vector<Vertex> vertices = ProcessVertices(meshNode);
        for (auto& vert : vertices) {
            vert.position = ConvertMatrixToGLMFormat(node->mTransformation) * glm::vec4(vert.position, 1.0f);
        }
        Ref<Mesh> mesh = CreateRef<Mesh>();
        mesh->SetName(meshNode->mName.C_Str());
        mesh->AddSurface(std::move(vertices), ProcessIndices(meshNode));
        //mesh->SetMaterial(ProcessMaterials(scene, meshNode));

        return mesh;
    }

    void ProcessNode(aiNode* node, const aiScene* scene) {
        for (uint32_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            g_meshes.push_back(ProcessMesh(mesh, node, scene));
        }
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    //Ref<Material> ProcessMaterials(const aiScene* scene, aiMesh* mesh) {
    //
    //    if (mesh->mMaterialIndex < 0)
    //        return nullptr;
    //
    //    aiString materialName;
    //    aiMaterial* materialNode = scene->mMaterials[mesh->mMaterialIndex];
    //    materialNode->Get(AI_MATKEY_NAME, materialName);
    //    const std::string& materialNameStr = std::string(materialName.C_Str());
    //    if (auto found = m_Materials.find(materialNameStr);
    //        found != m_Materials.end())
    //        return found->second;
    //
    //    Ref<Material> material = CreateRef<Material>();
    //    material->SetName(std::string(materialName.C_Str()));
    //
    //    aiString str;
    //    if (materialNode->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
    //        materialNode->GetTexture(aiTextureType_DIFFUSE, 0, &str);
    //        //Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
    //        //material->SetAlbedo(albedoTexture);
    //    }
    //
    //    m_Materials[materialNameStr] = material;
    //
    //    return material;
    //}
    //
    //Ref<Texture> ProcessTextures(const aiScene* scene, const std::string& path)
    //{
    //    if (String::BeginsWith(path, "*")) {
    //        uint32_t textureIndex = std::atoi(String::Split(path, '*')[1].c_str());
    //        const aiTexture* aiTexture = scene->GetEmbeddedTexture(path.c_str());
    //
    //        Ref<Texture> texture;
    //        //if (aiTexture->mHeight == 0)
    //        //    texture = CreateRef<Texture>((unsigned char*)aiTexture->pcData, static_cast<int>(aiTexture->mWidth));
    //
    //        return texture;
    //    }
    //
    //    std::string texturePath = modelDir + path;
    //    if (!FileSystem::FileExists(texturePath, true)) {
    //
    //        texturePath = "data/textures/Default.png";
    //    }
    //    return TextureManager::Get()->GetTexture(texturePath);
    //}

    void GrabSkeleton(std::vector<Node>& nodes, const aiNode* pNode, int parentIndex) {
        // Create the joint node
        Node node;
        node.name = /*Util::CopyConstChar(*/pNode->mName.C_Str();
        node.inverseBindTransform = Util::aiMatrix4x4ToGlm(pNode->mTransformation);
        node.parentIndex = parentIndex;

        // Determine the current node's index and push it
        int currentIndex = static_cast<int>(nodes.size());
        nodes.push_back(node);

        // Recursively process children using the current node's index as parentIndex
        for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
            GrabSkeleton(nodes, pNode->mChildren[i], currentIndex);
        }
    }
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

Model* AssetManager::LoadModel(const std::string& path, bool absolute) {
    g_meshes.clear();
    Model* model = new Model(path);

    Assimp::Importer importer;
    importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);

    auto importFlags =
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FixInfacingNormals |
        aiProcess_CalcTangentSpace;

    //modelDir = absolute ? path + "/../" : File::Root + path + "/../";
    //modelDir = path;
    const std::string filePath = absolute ? path : FileSystem::Root + path;
    //const std::string filePath = path;
    const aiScene* scene = importer.ReadFile(filePath, importFlags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::string assimpErrorMsg = std::string(importer.GetErrorString());
        std::string logMsg = "Failed to load model: " + assimpErrorMsg;
        //std::cout << logMsg << "\n";
        HZ_CORE_WARN(logMsg);
        return model;
    }

    ProcessNode(scene->mRootNode, scene);

    for (const auto& mesh : g_meshes) {
        model->AddMesh(mesh);
    }

    std::string name = scene->mRootNode->mName.C_Str();
    model->Name = name.substr(0, name.find_last_of("."));

    importer.FreeScene();
    return model;
}

Ref<SkinnedModel> AssetManager::LoadSkinnedModel(const std::string& path, bool absolute) {
    Ref<SkinnedModel> skinnedModel = CreateRef<SkinnedModel>();

    int totalVertexCount = 0;
    int baseVertexLocal = 0;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_LimitBoneWeights | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene) {
        std::cout << "Something fucked up loading your skinned model: " << path << "\n";
        std::cout << "Error: " << importer.GetErrorString() << "\n";
        return skinnedModel;
    }

    // Load bones
    int foundBoneCount = 0;
    for (int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* assimpMesh = scene->mMeshes[i];

        for (unsigned int j = 0; j < assimpMesh->mNumBones; j++) {
            const aiBone* bone = assimpMesh->mBones[j];
            std::string boneName = bone->mName.data;

            // If this bone isn't mapped yet, add it to the global list
            bool inserted = skinnedModel->m_boneMapping.find(boneName) != skinnedModel->m_boneMapping.end();
            if (!inserted) {

                // Map bone name to index
                unsigned int boneIndex = foundBoneCount++;
                skinnedModel->m_boneMapping[boneName] = boneIndex;

                // Store bone info
                glm::mat4 boneOffset = Util::aiMatrix4x4ToGlm(bone->mOffsetMatrix);
                skinnedModel->m_boneOffsets.push_back(boneOffset);
            }
        }
    }

    // Get vertex data
    for (int i = 0; i < scene->mNumMeshes; i++) {

        glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
        const aiMesh* assimpMesh = scene->mMeshes[i];
        int vertexCount = assimpMesh->mNumVertices;
        int indexCount = assimpMesh->mNumFaces * 3;
        std::string meshName = assimpMesh->mName.C_Str();
        std::vector<WeightedVertex> vertices;
        std::vector<uint32_t> indices;

        SkinnedMesh mesh;

        // Get vertices
        for (unsigned int j = 0; j < vertexCount; j++) {
            WeightedVertex vertex;
            vertex.position = { assimpMesh->mVertices[j].x, assimpMesh->mVertices[j].y, assimpMesh->mVertices[j].z };
            vertex.normal = { assimpMesh->mNormals[j].x, assimpMesh->mNormals[j].y, assimpMesh->mNormals[j].z };
            vertex.tangent = { assimpMesh->mTangents[j].x, assimpMesh->mTangents[j].y, assimpMesh->mTangents[j].z };
            vertex.uv = { assimpMesh->HasTextureCoords(0) ? glm::vec2(assimpMesh->mTextureCoords[0][j].x, assimpMesh->mTextureCoords[0][j].y) : glm::vec2(0.0f, 0.0f) };
            vertices.push_back(vertex);
            aabbMin.x = std::min(aabbMin.x, vertex.position.x);
            aabbMin.y = std::min(aabbMin.y, vertex.position.y);
            aabbMin.z = std::min(aabbMin.z, vertex.position.z);
            aabbMax.x = std::max(aabbMax.x, vertex.position.x);
            aabbMax.y = std::max(aabbMax.y, vertex.position.y);
            aabbMax.z = std::max(aabbMax.z, vertex.position.z);
        }
        // Get indices
        for (unsigned int j = 0; j < assimpMesh->mNumFaces; j++) {
            const aiFace& Face = assimpMesh->mFaces[j];
            indices.push_back(Face.mIndices[0]);
            indices.push_back(Face.mIndices[1]);
            indices.push_back(Face.mIndices[2]);
        }

        // Get vertex weights and bone IDs
        std::vector<unsigned int> influenceCount(vertices.size(), 0);

        for (unsigned int i = 0; i < assimpMesh->mNumBones; i++) {
            std::string boneName = assimpMesh->mBones[i]->mName.data;
            unsigned int boneIndex = skinnedModel->m_boneMapping[boneName];

            for (unsigned int j = 0; j < assimpMesh->mBones[i]->mNumWeights; j++) {
                unsigned int vertexIndex = assimpMesh->mBones[i]->mWeights[j].mVertexId;
                float weight = assimpMesh->mBones[i]->mWeights[j].mWeight;
                WeightedVertex& vertex = vertices[vertexIndex];

                if (influenceCount[vertexIndex] < 4) {
                    switch (influenceCount[vertexIndex]) {
                    case 0:
                        vertex.boneID.x = boneIndex;
                        vertex.weight.x = weight;
                        break;
                    case 1:
                        vertex.boneID.y = boneIndex;
                        vertex.weight.y = weight;
                        break;
                    case 2:
                        vertex.boneID.z = boneIndex;
                        vertex.weight.z = weight;
                        break;
                    case 3:
                        vertex.boneID.w = boneIndex;
                        vertex.weight.w = weight;
                        break;
                    }
                    influenceCount[vertexIndex]++;
                }
            }
        }

        // Ingore broken weights
        float threshold = 0.05f;
        for (unsigned int j = 0; j < vertices.size(); j++) {
            WeightedVertex& vertex = vertices[j];
            std::vector<float> validWeights;
            for (int i = 0; i < 4; ++i) {
                if (vertex.weight[i] < threshold) {
                    vertex.weight[i] = 0.0f;
                }
                else {
                    validWeights.push_back(vertex.weight[i]);
                }
            }
            float sum = std::accumulate(validWeights.begin(), validWeights.end(), 0.0f);
            int validIndex = 0;
            for (int i = 0; i < 4; ++i) {
                if (vertex.weight[i] > 0.0f) {
                    vertex.weight[i] = validWeights[validIndex] / sum;
                    validIndex++;
                }
            }
        }

        mesh.AddSurface(vertices, indices);

        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        //skinnedModel->AddMeshIndex(AssetManager::CreateSkinnedMesh(meshName, vertices, indices, baseVertexLocal, aabbMin, aabbMax));
        skinnedModel->AddMesh(mesh);
        totalVertexCount += vertices.size();
        baseVertexLocal += vertices.size();
    }
    //skinnedModel->SetVertexCount(totalVertexCount);

    // Recursively grab the skeleton
    std::vector<Node>& nodes = skinnedModel->m_nodes;
    GrabSkeleton(nodes, scene->mRootNode, -1);

    // Cleanup
    importer.FreeScene();

    return skinnedModel;
}

Ref<Texture> AssetManager::LoadTexture(const std::string& path) {
    if (m_texturesMap.find(path) == m_texturesMap.end())
        m_texturesMap.emplace(path, CreateRef<Texture>(path));
    return m_texturesMap.at(path);
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

    std::string content = FileSystem::ReadFile(path);
    json j = json::parse(content);

    Ref<Material> material = CreateRef<Material>();
    material->Path = path;
    material->Deserialize(j);
    g_loadLog.push_back(path);
    return material;
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
