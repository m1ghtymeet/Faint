#include "CMaterialRenderer.h"
#include <Debug/Assertion.h>
#include <Scene/Entity.h>
#include <Scene/Components/CModelRenderer.h>
#include <Core/String.h>
#include <AssetManagment/MeshManager.h>

Moon::MaterialRendererComp::MaterialRendererComp(Entity& p_owner) : AComponent(p_owner) {
    
	m_materials.resize(kMaxMaterialCount);
    UpdateMaterialList();
}

std::string Moon::MaterialRendererComp::GetName() {
    return "Material Renderer";
}

void Moon::MaterialRendererComp::AddMaterialByIndex(uint8_t p_index, Rendering::Material& p_material) {
    FT_CORE_ASSERT(p_index <= kMaxMaterialCount, "Invalid material index");
    m_materials[p_index] = std::make_shared<Rendering::Material>(p_material);

    if (p_material.GetShader())
        m_materials[p_index]->SetShader(p_material.GetShader());
}

void Moon::MaterialRendererComp::AddMaterialByIndex(uint8_t p_index, std::shared_ptr<Rendering::Material> p_material) {
    FT_CORE_ASSERT(p_index <= kMaxMaterialCount, "Invalid material index");
    m_materials[p_index] = p_material;
}

Moon::Rendering::Material* Moon::MaterialRendererComp::GetMaterialByIndex(uint8_t p_index) {
    return m_materials.at(p_index).get();
}

void Moon::MaterialRendererComp::RemoveMaterialByIndex(uint8_t p_index) {
    if (p_index < m_materials.size())
        m_materials[p_index].reset();
}

const Moon::MaterialRendererComp::MaterialList& Moon::MaterialRendererComp::GetMaterials() const {
    return m_materials;
}

void Moon::MaterialRendererComp::UpdateMaterialList() {
    if (auto modelRenderer = owner.GetComponent<Moon::MeshRendererComponent>(); modelRenderer && modelRenderer->GetModel()) {
        uint8_t materialIndex = 0;

        for (const std::string& materialName : modelRenderer->GetModel()->GetMaterialNames()) {
            m_materialNames[materialIndex++] = materialName;
        }

        for (uint8_t i = materialIndex; i < kMaxMaterialCount; i++)
            m_materialNames[i] = "";
    }
}

json Moon::MaterialRendererComp::Serialize() {
    BEGIN_SERIALIZE();
    if (owner.GetComponent<MeshRendererComponent>()) {
        MeshRendererComponent* meshRenderer = owner.GetComponent<MeshRendererComponent>();
        MaterialList materials = GetMaterials();
        for (uint32_t meshIndex : meshRenderer->GetModel()->GetMeshIndices()) {
            uint32_t materialIndex = AssetManagment::MeshManager::GetMeshByIndex(meshIndex)->materialIndex;
            const auto& currentMaterial = materials[materialIndex];
            j[std::to_string(materialIndex)] = currentMaterial->path;
        }
    }
    END_SERIALIZE();
}
