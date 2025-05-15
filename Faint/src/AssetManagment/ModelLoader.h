#pragma once

#include "Core/Base.h"

#include "Renderer/Types/Model.h"
#include "Renderer/Types/Mesh.h"
#include "Renderer/Types/Texture.h"

#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Faint {

	class Model;
	class SkinnedModel;

	class ModelLoader {
	public:
		ModelLoader();
		~ModelLoader();

		Ref<Model> LoadModel(const std::string& path, bool absolute = false);
		Ref<SkinnedModel> LoadSkinnedModel(const std::string& path, bool absolute = false);

	private:
		std::string modelDir;
		std::vector<Ref<Mesh>> m_meshes;
		//std::vector<Ref<SkinnedMesh>> m_SkinnedMeshes;
		std::map<std::string, Ref<Material>> m_Materials;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Ref<Mesh> ProcessMesh(aiMesh* mesh, aiNode* node, const aiScene* scene);

		std::vector<Vertex> ProcessVertices(aiMesh* mesh);
		std::vector<uint32_t> ProcessIndices(aiMesh* mesh);
		Ref<Material> ProcessMaterials(const aiScene* scene, aiMesh* mesh);
		Ref<Texture> ProcessTextures(const aiScene* scene, const std::string& path);

		static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
			glm::mat4 to;

			to[0][0] = from.a1; to[0][1] = from.b1;  to[0][2] = from.c1; to[0][3] = from.d1;
			to[1][0] = from.a2; to[1][1] = from.b2;  to[1][2] = from.c2; to[1][3] = from.d2;
			to[2][0] = from.a3; to[2][1] = from.b3;  to[2][2] = from.c3; to[2][3] = from.d3;
			to[3][0] = from.a4; to[3][1] = from.b4;  to[3][2] = from.c4; to[3][3] = from.d4;

			return to;
		}
	};
}