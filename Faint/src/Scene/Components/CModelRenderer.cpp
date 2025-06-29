#include "CModelRenderer.h"

Faint::MeshRendererComponent::MeshRendererComponent(Entity& p_owner) : AComponent(p_owner) {

}

void Faint::MeshRendererComponent::SetModel(Model* p_model) {
    m_model = p_model;
}

Model* Faint::MeshRendererComponent::GetModel() const {
    return m_model;
}

json Faint::MeshRendererComponent::Serialize() {
    BEGIN_SERIALIZE();
    SERIALIZE_VAL(ModelPath);
    if (m_model) {
        j["Model"] = m_model->Serialize();
    }
    END_SERIALIZE();
}

void Faint::MeshRendererComponent::Deserialize(const json& j) {
    ModelPath = j["ModelPath"];
    if (ModelPath.empty()) {
        if (j.contains("Model")) {
            auto& res = j["Model"];
            m_model->Deserialize(res);
        }
    }
    else {
        m_model = AssetManager::LoadModel(ModelPath);
    }
}
