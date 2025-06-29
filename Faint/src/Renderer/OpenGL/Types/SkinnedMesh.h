#pragma once
#include "AssetManagment/Serializable.h"
#include "Common/Types.h"
#include "Renderer/Types/Material.h"
#include <glad/glad.h>

namespace Faint {
	struct SkinnedMesh : public ISerializable {
	public:
		SkinnedMesh();
		//SkinnedMesh(const std::string path);
		~SkinnedMesh() = default;

		void AddSurface(std::vector<WeightedVertex> vertices, std::vector<uint32_t> indices);
		std::vector<WeightedVertex>& GetVertices();
		std::vector<uint32_t>& GetIndices();

		void Draw(bool bindMaterial);

		json Serialize() override;
		void Deserialize(const json& j) override;
	private:
		GLuint vao, vbo, ebo;

		Material* m_material = nullptr;
		std::vector<WeightedVertex> m_vertices;
		std::vector<uint32_t> m_indices;
	};
}