#pragma once

#include "AComponent.h"
#include <Renderer/Types/Material.h>

#include <vector>

constexpr uint8_t kMaxMaterialCount = 0xFF;

namespace Moon {
    class Entity;

    class MaterialRendererComp : public AComponent {
    public:
        using MaterialList = std::vector<std::shared_ptr<Rendering::Material>>;
        MaterialRendererComp(Entity& p_owner);

        std::string GetName() override;

        void AddMaterialByIndex(uint8_t p_index, Rendering::Material& p_material);
        void AddMaterialByIndex(uint8_t p_index, std::shared_ptr<Rendering::Material> p_material);

        Rendering::Material* GetMaterialByIndex(uint8_t p_index);

        void RemoveMaterialByIndex(uint8_t p_index);

        const MaterialList& GetMaterials() const;

        void UpdateMaterialList();

        json Serialize();

    private:
        MaterialList m_materials;
        std::array<std::string, kMaxMaterialCount> m_materialNames;
	};
}