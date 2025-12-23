#include "CSkinnedModelRenderer.h"

Moon::SkinnedMeshRendererComp::SkinnedMeshRendererComp(Entity& p_owner) :
	AComponent(p_owner) {
}

std::string Moon::SkinnedMeshRendererComp::GetName() {
	return "Skinned Mesh Renderer";
}

void Moon::SkinnedMeshRendererComp::SetModel(Rendering::SkinnedModel* p_model) {
	m_model = p_model;
}

Moon::Rendering::SkinnedModel* Moon::SkinnedMeshRendererComp::GetModel() const {
	return m_model;
}

json Moon::SkinnedMeshRendererComp::Serialize() {
	BEGIN_SERIALIZE();
	END_SERIALIZE();
}

void Moon::SkinnedMeshRendererComp::Deserialize(const json& j) {

}
