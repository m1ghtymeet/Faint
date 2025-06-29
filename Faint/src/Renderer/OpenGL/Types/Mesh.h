#pragma once
#include "Core/Base.h"
#include "Math/Math.h"
#include "Math/AABB.h"
#include "AssetManagment/Serializable.h"
#include "Common/Types.h"
#include "Renderer/Types/VertexArray.h"
#include "AssetManagment/Resource.h"

namespace Faint {
	struct Material;
	class Shader;

	class Mesh : public Resource, ISerializable {
	public:
		Mesh() = default;
		~Mesh() = default;

		void AddSurface(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		std::vector<Vertex>& GetVertices();
		std::vector<uint32_t>& GetIndices();

		Ref<Material> GetMaterial() const;
		void SetMaterial(Ref<Material> material);

		void Bind() const;
		void Draw(Shader* shader, bool bindMaterial = true);
		void DebugDraw();

		const std::string& GetName() { return m_name; }
		void SetName(const std::string& name);

		inline AABB GetAABB() const { return m_AABB; }

		json Serialize() override;
		void Deserialize(const json& j) override;

	private:
		Ref<Material> m_Material = nullptr;
		std::vector<uint32_t> m_indices;
		std::vector<Vertex> m_vertices;
		std::string m_name = "undefined";

		uint32_t m_indicesCount;
		uint32_t m_verticesCount;

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<VertexArray> m_vertexArray;
		Ref<IndexBuffer> m_elementBuffer;

		void SetupMesh();

		AABB m_AABB;
		void CalculateAABB();
	};
}