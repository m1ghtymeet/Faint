#include "hzpch.h"
#include "Prefab.h"

#include "Scene/Components/ParentComponent.h"

namespace Faint {

    Ref<Prefab> Prefab::CreatePrefabFromEntity(Entity entity) {
        
        auto& name = entity.GetComponent<NameComponent>().name;
    
        Ref<Prefab> prefab = CreateRef<Prefab>();
        prefab->DisplayName = name;
        prefab->EntityWalker(entity);
        entity.GetComponent<TransformComponent>() = TransformComponent();
        prefab->Root = entity;
        return prefab;
    }

    Ref<Prefab> Prefab::New(const std::string& path) {

        Ref<Prefab> newPrefab = CreateRef<Prefab>();
        newPrefab->Path = path;

        if (FileSystem::FileExists(path, false)) {
            
            std::string prefabTextContent = FileSystem::ReadFile(path);

            if (!prefabTextContent.empty()) {
                newPrefab->Deserialize(json::parse(prefabTextContent));
            }
        }
        
        return newPrefab;
    }

    void Prefab::EntityWalker(Entity entity) {

        Root = entity;
        entity.GetComponent<NameComponent>().isPrefab = true;
        Entities.push_back(entity);

        for (const Entity& e : entity.GetComponent<ParentComponent>().Children) {
            EntityWalker(e);
        }
    }
}