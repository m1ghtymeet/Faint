#pragma once

#include "../Math/Math.h"
#include "Types/Mesh.h"
#include "Types/Material.h"
#include "Shader.h"

namespace Faint {

	struct RenderMesh {
		Ref<Mesh> mesh;
		Matrix4 transform;
		int32_t entityId;
		Matrix4 previousMatrix;
	};

	class RenderList {
	private:
		std::unordered_map<Ref<Material>, std::vector<RenderMesh>> m_RenderList;
	public:
		RenderList()
		{
			this->m_RenderList = std::unordered_map<Ref<Material>, std::vector<RenderMesh>>();
		}

		void AddToRenderList(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityId = -1, const Matrix4& previousMatrix = Matrix4(0.0f))
		{
			Ref<Material> material = mesh->GetMaterial();

			if (m_RenderList.find(material) == m_RenderList.end())
				m_RenderList[material] = std::vector<RenderMesh>();

			m_RenderList[material].push_back({ std::move(mesh), std::move(transform), entityId, previousMatrix });
		}

		void Flush(Shader* shader, bool depthOnly = false) {
			shader->Bind();
			for (auto& i : m_RenderList)
			{
				if (!depthOnly)
					i.first->Bind(shader);

				for (auto& m : i.second)
				{
					if (!depthOnly)
						shader->SetInt("aEntityID", m.entityId + 1);

					shader->SetMat4("modelMatrix", m.transform);
					m.mesh->Draw(shader);
				}
			}

			m_RenderList.clear();
		}
	};
}