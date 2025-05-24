#include "hzpch.h"
#include "Mesh.h"
#include "Material.h"

#include "Renderer/RenderCommand.h"
#include "AssetManagment/AssetManager.h"

namespace Faint {
	void Mesh::AddSurface(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {

		this->m_vertices = vertices;
		this->m_indices = indices;

		SetupMesh();
		CalculateAABB();

		if (m_Material == nullptr) {
			m_Material = CreateRef<Material>();
		}

		m_indicesCount = static_cast<uint32_t>(m_indices.size());
		m_verticesCount = static_cast<uint32_t>(m_vertices.size());
	}
	std::vector<Vertex>& Mesh::GetVertices() {
		return m_vertices;
	}
	std::vector<uint32_t>& Mesh::GetIndices() {
		return m_indices;
	}
	Ref<Material> Mesh::GetMaterial() const {
		return m_Material;
	}
	void Mesh::SetMaterial(Ref<Material> material) {
		m_Material = material;
	}
	void Mesh::Bind() const {
		m_vertexArray->Bind();
	}
	void Mesh::Draw(Shader* shader, bool bindMaterial) {
		if (bindMaterial) {
			m_Material->Bind(shader);
		}

		m_vertexArray->Bind();
		RenderCommand::DrawElements(RendererEnum::TRIANGLES, m_indicesCount, RendererEnum::UINT, 0);
		glBindVertexArray(0);
	}
	void Mesh::DebugDraw() {
		
		m_vertexArray->Bind();
		RenderCommand::DrawElements(RendererEnum::TRIANGLES, m_indicesCount, RendererEnum::UINT, 0);
	}
	void Mesh::SetupMesh() {

		m_vertexArray = VertexArray::Create();
		m_vertexBuffer = VertexBuffer::Create(m_vertices/*static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex))*/);
		m_elementBuffer = IndexBuffer::Create(m_indices/*, m_indices.size() * sizeof(unsigned int)*/);

		BufferLayout layout = {
			{ ShaderDataType::FLOAT3, "aPos" },
			{ ShaderDataType::FLOAT3, "aNormal" },
			{ ShaderDataType::FLOAT2, "aTexCoord" },
			{ ShaderDataType::FLOAT3, "aTangent" },
		};

		m_vertexBuffer->SetLayout(layout);
		m_vertexArray->AddVertexBuffer(m_vertexBuffer);
		m_vertexArray->AddIndexBuffer(m_elementBuffer);

		//VertexBufferLayout bufferLayout = VertexBufferLayout();
		//bufferLayout.Push<float>(3); // Position
		//bufferLayout.Push<float>(2); // UV
		//bufferLayout.Push<float>(3); // Normal
		//bufferLayout.Push<float>(3); // Tangent
		//bufferLayout.Push<float>(3); // Bitangent

		//m_vertexArray->AddBuffer(*m_VertexBuffer, bufferLayout);
		m_vertexArray->UnBind();
	}

	void Mesh::CalculateAABB()
	{
		// Initialize AABB min and max with first vertex
		glm::vec3 aabbMin = m_vertices[0].position;
		glm::vec3 aabbMax = m_vertices[0].position;

		// Calculate AABB by iterating over all vertices
		for (const Vertex& v : m_vertices) {
			aabbMin = glm::min(aabbMin, v.position);
			aabbMax = glm::max(aabbMax, v.position);
		}

		m_AABB = AABB(aabbMin, aabbMax);
	}

	json Mesh::Serialize()
	{
		BEGIN_SERIALIZE();

		if (m_Material)
		{
			j["Material"] = m_Material->Serialize();
		}

		j["Indices"] = m_indices;

		json v;
		for (uint32_t i = 0; i < m_vertices.size(); i++)
		{
			v["Position"]["x"] = m_vertices[i].position.x;
			v["Position"]["y"] = m_vertices[i].position.y;
			v["Position"]["z"] = m_vertices[i].position.z;

			v["UV"]["x"] = m_vertices[i].uv.x;
			v["UV"]["y"] = m_vertices[i].uv.y;

			v["Normal"]["x"] = m_vertices[i].normal.x;
			v["Normal"]["y"] = m_vertices[i].normal.y;
			v["Normal"]["z"] = m_vertices[i].normal.z;

			v["Tangent"]["x"] = m_vertices[i].tangent.x;
			v["Tangent"]["y"] = m_vertices[i].tangent.y;
			v["Tangent"]["z"] = m_vertices[i].tangent.z;

			//v["Bitangent"]["x"] = m_Vertices[i].bitangent.x;
			//v["Bitangent"]["y"] = m_Vertices[i].bitangent.y;
			//v["Bitangent"]["z"] = m_Vertices[i].bitangent.z;

			j["Vertices"][i] = v;
		}

		END_SERIALIZE();
	}

	bool Mesh::Deserialize(const json& j) {

		bool loadedMaterialFile = false;
		if (j.contains("Material") && j["Material"].contains("Path"))
		{
			const std::string materialPath = j["Material"]["Path"];
			if (!materialPath.empty())
			{
				Ref<Material> newMaterial = AssetManager::LoadMaterial(materialPath);
				m_Material = newMaterial;
				loadedMaterialFile = true;
			}
		}

		if (!loadedMaterialFile && j.contains("Material"))
		{
			m_Material = CreateRef<Material>();
			m_Material->Deserialize(j["Material"]);
		}

		m_indices.reserve(j["Indices"].size());
		for (auto& i : j["Indices"])
		{
			m_indices.push_back(i);
		}

		m_indicesCount = static_cast<uint32_t>(m_indices.size());

		auto result = std::async(std::launch::async, [&]()
			{
				std::vector<Vertex> vertices;
				for (auto& v : j["Vertices"])
				{
					Vertex vertex;
					try {
						DESERIALIZE_VEC2(v["UV"], vertex.uv)
					}
					catch (std::exception& /*e*/) {
						vertex.uv = { 0.0, 0.0 };
					}
					DESERIALIZE_VEC3(v["Position"], vertex.position)
					DESERIALIZE_VEC3(v["Normal"], vertex.normal)
					DESERIALIZE_VEC3(v["Tangent"], vertex.tangent)
					//DESERIALIZE_VEC3(v["Bitangent"], vertex.bitangent)
					vertices.push_back(vertex);
				}

				return vertices;
			}
		);

		m_vertices = result.get();
		CalculateAABB();

		SetupMesh();
		return true;
	}

	bool Mesh::SerializeYaml(YAML::Emitter& emit)
	{
		//emit << YAML::Key << "Mesh" << YAML::Value;
		emit << YAML::BeginMap;
		{
			emit << YAML::Key << "UUID" << YAML::Value << static_cast<uint64_t>(id);

			if (m_Material)
			{
				m_Material->SerializeYaml(emit);
			}
		}
		emit << YAML::EndMap;

		/*json v;
		for (uint32_t i = 0; i < m_vertices.size(); i++)
		{
			v["Position"]["x"] = m_vertices[i].position.x;
			v["Position"]["y"] = m_vertices[i].position.y;
			v["Position"]["z"] = m_vertices[i].position.z;

			v["UV"]["x"] = m_vertices[i].uv.x;
			v["UV"]["y"] = m_vertices[i].uv.y;

			v["Normal"]["x"] = m_vertices[i].normal.x;
			v["Normal"]["y"] = m_vertices[i].normal.y;
			v["Normal"]["z"] = m_vertices[i].normal.z;

			v["Tangent"]["x"] = m_vertices[i].tangent.x;
			v["Tangent"]["y"] = m_vertices[i].tangent.y;
			v["Tangent"]["z"] = m_vertices[i].tangent.z;

			//v["Bitangent"]["x"] = m_Vertices[i].bitangent.x;
			//v["Bitangent"]["y"] = m_Vertices[i].bitangent.y;
			//v["Bitangent"]["z"] = m_Vertices[i].bitangent.z;

			j["Vertices"][i] = v;
		}*/

		return true;
	}
	bool Mesh::DeserializeYaml(YAML::Node& in)
	{
		bool loadedMaterialFile = false;
		if (in["Model"]["Material"] && in["Model"]["Material"]["Path"])
		{
			const std::string materialPath = in["Model"]["Material"]["Path"].as<std::string>();
			if (!materialPath.empty())
			{
				Ref<Material> newMaterial = AssetManager::LoadMaterial(materialPath);
				m_Material = newMaterial;
				loadedMaterialFile = true;
			}
		}

		if (!loadedMaterialFile && in["Model"]["Material"])
		{
			m_Material = CreateRef<Material>();
			m_Material->Deserialize(in["Model"]["Material"].as<std::string>());
		}

		m_indices.reserve(in["Model"]["Mesh"]["Indices"].size());
		//for (auto& i : in["Indices"])
		//{
		//	m_indices.push_back(i);
		//}

		m_indicesCount = static_cast<uint32_t>(m_indices.size());

		/*auto result = std::async(std::launch::async, [&]()
			{
				std::vector<Vertex> vertices;
				for (auto& v : j["Vertices"])
				{
					Vertex vertex;
					try {
						DESERIALIZE_VEC2(v["UV"], vertex.uv)
					}
					catch (std::exception& /*e*/ /*) {
						vertex.uv = { 0.0, 0.0 };
					}
					DESERIALIZE_VEC3(v["Position"], vertex.position)
						DESERIALIZE_VEC3(v["Normal"], vertex.normal)
						DESERIALIZE_VEC3(v["Tangent"], vertex.tangent)
						//DESERIALIZE_VEC3(v["Bitangent"], vertex.bitangent)
						vertices.push_back(vertex);
				}

				return vertices;
			}
		);

		m_vertices = result.get();
		CalculateAABB();

		SetupMesh();*/
		return true;
	}
}