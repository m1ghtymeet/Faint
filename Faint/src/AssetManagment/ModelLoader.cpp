#include "hzpch.h"
#include "ModelLoader.h"

#include "Core/String.h"

#include "Renderer/Types/Material.h"
#include "Renderer/Types/Texture.h"
#include "FileSystem/FileSystem.h"

namespace Faint {

    ModelLoader::ModelLoader() { }
    ModelLoader::~ModelLoader() { }

    Ref<Model> ModelLoader::LoadModel(const std::string& path, bool absolute) {
        
        m_meshes.clear();
        Ref<Model> model = CreateRef<Model>(path);

        Assimp::Importer importer;
        importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);

        auto importFlags =
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FixInfacingNormals |
            aiProcess_CalcTangentSpace;

        //modelDir = absolute ? path + "/../" : File::Root + path + "/../";
        modelDir = path;
        const std::string filePath = absolute ? path : FileSystem::Root + path;
        //const std::string filePath = path;
        const aiScene* scene = importer.ReadFile(filePath, importFlags);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::string assimpErrorMsg = std::string(importer.GetErrorString());
            std::string logMsg = "Failed to load model: " + assimpErrorMsg;
            std::cout << logMsg << "\n";
            return model;
        }

        ProcessNode(scene->mRootNode, scene);
        
        for (const auto& mesh : m_meshes)
        {
            model->AddMesh(mesh);
        }

        std::string name = scene->mRootNode->mName.C_Str();
        model->Name = name.substr(0, name.find_last_of("."));

        importer.FreeScene();
        return model;
    }

    void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene) {
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(ProcessMesh(mesh, node, scene));
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    Ref<Mesh> ModelLoader::ProcessMesh(aiMesh* meshNode, aiNode* node, const aiScene* scene) {

        auto vertices = ProcessVertices(meshNode);
        for (auto& vert : vertices) {
            vert.position = ConvertMatrixToGLMFormat(node->mTransformation) * glm::vec4(vert.position, 1.0f);
        }

        Ref<Mesh> mesh = CreateRef<Mesh>();
        mesh->AddSurface(std::move(vertices), ProcessIndices(meshNode));
        mesh->SetMaterial(ProcessMaterials(scene, meshNode));

        return mesh;
    }

    Ref<Material> ModelLoader::ProcessMaterials(const aiScene* scene, aiMesh* mesh) {
        
        if (mesh->mMaterialIndex < 0)
            return nullptr;

        aiString materialName;
        aiMaterial* materialNode = scene->mMaterials[mesh->mMaterialIndex];
        materialNode->Get(AI_MATKEY_NAME, materialName);
        const std::string& materialNameStr = std::string(materialName.C_Str());
        if (auto found = m_Materials.find(materialNameStr);
            found != m_Materials.end())
            return found->second;

        Ref<Material> material = CreateRef<Material>();
        material->SetName(std::string(materialName.C_Str()));

        aiString str;
        if (materialNode->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            materialNode->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
            material->SetAlbedo(albedoTexture);
        }

        m_Materials[materialNameStr] = material;

        return material;
    }

    Ref<Texture> ModelLoader::ProcessTextures(const aiScene* scene, const std::string& path)
    {
        if (String::BeginsWith(path, "*")) {
            uint32_t textureIndex = std::atoi(String::Split(path, '*')[1].c_str());
            const aiTexture* aiTexture = scene->GetEmbeddedTexture(path.c_str());

            Ref<Texture> texture;
            //if (aiTexture->mHeight == 0)
            //    texture = CreateRef<Texture>((unsigned char*)aiTexture->pcData, static_cast<int>(aiTexture->mWidth));
            
            return texture;
        }

        std::string texturePath = modelDir + path;
        if (!FileSystem::FileExists(texturePath, true)) {

            texturePath = "data/textures/Default.png";
        }
        return TextureManager::Get()->GetTexture(texturePath);
    }

    std::vector<Vertex> ModelLoader::ProcessVertices(aiMesh* mesh) {
        
        auto vertices = std::vector<Vertex>();
        vertices.reserve(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex{};

            glm::vec3 current;

            // Position
            vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

            // Tangents
            if (mesh->mTangents)
            {
                vertex.tangent = {
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z
                };
            }

            if (mesh->mBitangents)
            {
                //vertex.bitangent = {
                //    mesh->mBitangents[i].x,
                //    mesh->mBitangents[i].z,
                //    mesh->mBitangents[i].y
                //};
            }

            vertex.uv = glm::vec2(0.0f, 0.0f);

            // Does it contain UVs?
            if (mesh->mTextureCoords[0])
            {
                vertex.uv = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            vertices.push_back(std::move(vertex));
        }

        return vertices;
    }

    std::vector<uint32_t> ModelLoader::ProcessIndices(aiMesh* mesh) {

        auto indices = std::vector<uint32_t>();
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return indices;
    }
}