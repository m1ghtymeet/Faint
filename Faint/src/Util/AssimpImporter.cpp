#include "AssimpImporter.h"
#include <Debug/Log.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace Moon::AssimpImporter {
	glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	void GrabSkeleton(std::vector<Rendering::Node>& nodes, const aiNode* pNode, int parentIndex) {
		// Create the joint node
		Rendering::Node node;
		node.name = pNode->mName.C_Str();
		node.inverseBindTransform = aiMatrix4x4ToGlm(pNode->mTransformation);
		node.parentIndex = parentIndex;
		// Determine the current node's index and push it
		int currentIndex = static_cast<int>(nodes.size());
		nodes.push_back(node);
		// Recursively process children using the current node's index as parentIndex
		for (unsigned int i = 0; i < pNode->mNumChildren; i++)
			GrabSkeleton(nodes, pNode->mChildren[i], currentIndex);
	}
	
}

Moon::ModelData Moon::AssimpImporter::ImportFbx(const std::string& filepath) {
	Moon::ModelData model;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FixInfacingNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_FlipUVs
	);
	if (!scene || !scene->HasMeshes()) {
		HZ_CORE_ERROR("Assimp failed to load model: " + filepath);
		return {};
	}
	model.name = std::filesystem::path(filepath).stem().string();
	model.meshCount = scene->mNumMeshes;
	model.timestamp = std::time(nullptr);

	model.meshes.clear();
	model.meshes.reserve(model.meshCount);

	// --- Process Materials ---
	std::unordered_map<uint32_t, uint32_t> materialRemap;
	model.materialNames.clear();

	for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];
		if (!material) continue;

		aiString name;
		std::string materialName = "undefined";
		if (AI_SUCCESS == material->Get(AI_MATKEY_NAME, name))
			materialName = name.C_Str();

		materialRemap[i] = static_cast<uint32_t>(model.materialNames.size());
		model.materialNames.push_back(materialName);
	}

	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		MeshData meshData;
		meshData.name = mesh->mName.C_Str();
		meshData.vertexCount = mesh->mNumVertices;
		meshData.indexCount = mesh->mNumFaces * 3;

		if (materialRemap.contains(mesh->mMaterialIndex - 1))
			meshData.materialIndex = materialRemap[mesh->mMaterialIndex - 1];
		else
			meshData.materialIndex = 0; // fallback

		meshData.vertices.resize(mesh->mNumVertices);
		for (uint32_t v = 0; v < mesh->mNumVertices; v++) {
			Moon::Rendering::Vertex vertex;

			vertex.position = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
			if (mesh->HasNormals())
				vertex.normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
			if (mesh->HasTextureCoords(0))
				vertex.uv = glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
			else
				vertex.uv = glm::vec2(0.0f);

			if (mesh->HasTangentsAndBitangents())
				vertex.tangent = glm::vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
			else
				vertex.tangent = glm::vec3(0.0f);

			meshData.aabbMin = glm::min(meshData.aabbMin, vertex.position);
			meshData.aabbMax = glm::max(meshData.aabbMax, vertex.position);

			meshData.vertices[v] = vertex;
		}

		meshData.indices.reserve(meshData.indexCount);
		for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
			aiFace face = mesh->mFaces[f];
			for (uint32_t idx = 0; idx < face.mNumIndices; idx++)
				meshData.indices.push_back(face.mIndices[idx]);
		}

		model.aabbMin = glm::min(model.aabbMin, meshData.aabbMin);
		model.aabbMax = glm::min(model.aabbMax, meshData.aabbMax);

		model.meshes.push_back(std::move(meshData));
	}

	return model;
}

Moon::Rendering::SkinnedModelData Moon::AssimpImporter::ImportSkinnedFbx(const std::string& filepath) {
	Moon::Rendering::SkinnedModelData modelData;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		filepath.c_str(),
		aiProcess_LimitBoneWeights | aiProcess_Triangulate |
		aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	);

	if (!scene) {
		std::cout << "Assimp failed to load skinned model: " << filepath << "\n";
		return {};
	}

	modelData.name = std::filesystem::path(filepath).stem().string();
	modelData.meshes.resize(scene->mNumMeshes);
	modelData.timestamp = 0; // TODO: GetLastModifiedTime
	modelData.vertexCount = 0;
	modelData.indexCount = 0;

	// Load bones
	int foundBoneCount = 0;
	for (int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* assimpMesh = scene->mMeshes[i];

		for (unsigned int j = 0; j < assimpMesh->mNumBones; j++) {
			const aiBone* bone = assimpMesh->mBones[j];
			std::string boneName = bone->mName.data;

			bool inserted = modelData.boneMapping.find(boneName) != modelData.boneMapping.end();
			if (!inserted) {
				unsigned int boneIndex = foundBoneCount++;
				modelData.boneMapping[boneName] = boneIndex;

				glm::mat4 boneOffset = aiMatrix4x4ToGlm(bone->mOffsetMatrix);
				modelData.boneOffsets.push_back(boneOffset);
			}
		}
	}

	// Recurively grab the skeleton
	GrabSkeleton(modelData.nodes, scene->mRootNode, -1);

	// Get vertex data
	int localBaseVertex = 0;
	for (int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* assimpMesh = scene->mMeshes[i];

		Moon::Rendering::SkinnedMeshData& meshData = modelData.meshes[i];
		meshData.aabbMin = glm::vec3(std::numeric_limits<float>::max());
		meshData.aabbMax = glm::vec3(-std::numeric_limits<float>::max());
		meshData.vertexCount = assimpMesh->mNumVertices;
		meshData.indexCount = assimpMesh->mNumFaces * 3;
		meshData.name = assimpMesh->mName.C_Str();
		meshData.localBaseVertex = localBaseVertex;
		meshData.vertices.reserve(meshData.vertexCount);
		meshData.indices.reserve(meshData.indexCount);
		
		// Get vertices
		for (unsigned int j = 0; j < meshData.vertexCount; j++) {
			Moon::Rendering::WeightedVertex vertex;
			vertex.position = { assimpMesh->mVertices[j].x, assimpMesh->mVertices[j].y, assimpMesh->mVertices[j].z };
			vertex.normal = { assimpMesh->mNormals[j].x, assimpMesh->mNormals[j].y, assimpMesh->mNormals[j].z };
			vertex.tangent = { assimpMesh->mTangents[j].x, assimpMesh->mTangents[j].y, assimpMesh->mTangents[j].z };
			vertex.uv = { assimpMesh->HasTextureCoords(0) ? glm::vec2(assimpMesh->mTextureCoords[0][j].x, assimpMesh->mTextureCoords[0][j].y) : glm::vec2(0.0f, 0.0f) };
			meshData.vertices.push_back(vertex);
			meshData.aabbMin.x = glm::min(meshData.aabbMin.x, vertex.position.x);
			meshData.aabbMin.y = glm::min(meshData.aabbMin.y, vertex.position.y);
			meshData.aabbMin.z = glm::min(meshData.aabbMin.z, vertex.position.z);
			meshData.aabbMax.x = glm::min(meshData.aabbMax.x, vertex.position.x);
			meshData.aabbMax.y = glm::min(meshData.aabbMax.y, vertex.position.y);
			meshData.aabbMax.z = glm::min(meshData.aabbMax.z, vertex.position.z);
		}
		// Get indices
		for (unsigned int j = 0; j < assimpMesh->mNumFaces; j++) {
			const aiFace& face = assimpMesh->mFaces[j];
			meshData.indices.push_back(face.mIndices[0]);
			meshData.indices.push_back(face.mIndices[1]);
			meshData.indices.push_back(face.mIndices[2]);
		}

		// Get vertex weights and bone IDs
		std::vector<unsigned int> influenceCount(meshData.vertices.size(), 0);

		for (unsigned int i = 0; i < assimpMesh->mNumBones; i++) {
			std::string boneName = assimpMesh->mBones[i]->mName.data;
			unsigned int boneIndex = modelData.boneMapping[boneName];

			for (unsigned int j = 0; j < assimpMesh->mBones[i]->mNumWeights; j++) {
				unsigned int vertexIndex = assimpMesh->mBones[i]->mWeights[j].mVertexId;
				float weight = assimpMesh->mBones[i]->mWeights[j].mWeight;
				Rendering::WeightedVertex& vertex = meshData.vertices[vertexIndex];

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
		for (unsigned int j = 0; j < meshData.vertices.size(); j++) {
			Rendering::WeightedVertex& vertex = meshData.vertices[j];
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
		localBaseVertex += meshData.vertices.size();
		modelData.vertexCount += meshData.vertices.size();
		modelData.indexCount += meshData.indices.size();
	}

	importer.FreeScene();
	return modelData;
}
