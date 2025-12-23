#include "CModelRenderer.h"
#include <Core/GlobalLocator.h>
#include <Scene/Entity.h>
#include <Scene/Components/CMaterialRenderer.h>
#include <FileSystem/FileSystem.h>
#include <AssetManagment/ModelManager.h>
#include <Util/PlatformUtil.h>

Moon::MeshRendererComponent::MeshRendererComponent(Entity& p_owner) : AComponent(p_owner) {

    m_modelChangedEvent += [this] {
        if (auto materialRenderer = owner.GetComponent<MaterialRendererComp>())
            materialRenderer->UpdateMaterialList();
    };
}

void Moon::MeshRendererComponent::SetModel(Model* p_model) {
    m_model = p_model;
    m_modelChangedEvent.Invoke();
}

Moon::Model* Moon::MeshRendererComponent::GetModel() const {
    return m_model;
}

void Moon::MeshRendererComponent::SetMesh(Mesh* p_mesh, int p_meshIndex) {
    m_model = nullptr;
    m_meshes = { p_mesh };
    m_meshIndices = { p_meshIndex };
    m_modelChangedEvent.Invoke();
}

void Moon::MeshRendererComponent::AddMesh(Mesh* p_mesh, int p_meshIndex) {
    m_model = nullptr;
    m_meshes.push_back(p_mesh);
    m_meshIndices.push_back(p_meshIndex);
    m_modelChangedEvent.Invoke();
}

void Moon::MeshRendererComponent::ClearMeshes() {
    m_model = nullptr;
    m_meshes.clear();
    m_meshIndices.clear();
    m_modelChangedEvent.Invoke();
}

const std::vector<Moon::Mesh*>& Moon::MeshRendererComponent::GetMeshes() const {
    return m_meshes;
}

json Moon::MeshRendererComponent::Serialize() {
    BEGIN_SERIALIZE();
    SERIALIZE_VAL(ModelPath);
    END_SERIALIZE();
}

void Moon::MeshRendererComponent::Deserialize(const json& j) {
    if (j.contains("ModelPath")) {
        std::filesystem::path modelPath = j["ModelPath"];
        if (!modelPath.empty()) {
            ModelPath = modelPath.is_absolute()
                ? modelPath.string()
                : (modelPath.string().rfind("data", 0) == 0 ? modelPath.string() : FileSystem::RelativeToAbsolute(modelPath.string()));

            if (FileSystem::FileExists(ModelPath, true)) {
                Model* model = FTSERVICE(AssetManagment::ModelManager).LoadResource(ModelPath);
                SetModel(model);
            }
            else {
                Moon::MessageBox message(
                    "Model file not found",
                    "Cannot find model at: " + ModelPath,
                    Moon::MessageBox::EMessageType::ERROR
                );
                return;
            }
        }
    }
    owner.AddComponent<MaterialRendererComp>();
}
