#include "SkinnedMesh.h"
#include "AssetManagment/AssetManager.h"

namespace Faint {
	SkinnedMesh::SkinnedMesh() {
		m_material->m_Albedo = AssetManager::LoadTexture("C:/Users/Atmosfer-PC/Documents/Engine Projects/Example/Assets/textures/Ceiling2_ALB.png");
	}

	void SkinnedMesh::AddSurface(std::vector<WeightedVertex> vertices, std::vector<uint32_t> indices) {
		m_vertices = vertices;
		m_indices = indices;
		if (vao) {
			glDeleteVertexArrays(1, &vao);
		}

		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo);
		glCreateBuffers(1, &ebo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(WeightedVertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
	
		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WeightedVertex), (void*)offsetof(WeightedVertex, position));
		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WeightedVertex), (void*)offsetof(WeightedVertex, normal));
		glEnableVertexAttribArray(2); // uv
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(WeightedVertex), (void*)offsetof(WeightedVertex, uv));
		glEnableVertexAttribArray(3); // tangent
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(WeightedVertex), (void*)offsetof(WeightedVertex, tangent));
		glEnableVertexAttribArray(4); // boneIds
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(WeightedVertex), (void*)offsetof(WeightedVertex, boneID));
		glEnableVertexAttribArray(5); // weights
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(WeightedVertex), (void*)offsetof(WeightedVertex, weight));
	
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		if (m_material == nullptr)
			m_material = new Material();
	}

	std::vector<WeightedVertex>& SkinnedMesh::GetVertices() {
		return m_vertices;
	}

	std::vector<uint32_t>& SkinnedMesh::GetIndices() {
		return m_indices;
	}

	void SkinnedMesh::Draw(bool bindMaterial) {
		if (bindMaterial) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_material->m_Albedo->GetID());
		}
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, (int)m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	json SkinnedMesh::Serialize() {
		BEGIN_SERIALIZE();
		if (m_material) {
			j["MaterialPath"] = m_material->Path;
		}
		END_SERIALIZE();
	}

	void SkinnedMesh::Deserialize(const json& j) {

	}
}