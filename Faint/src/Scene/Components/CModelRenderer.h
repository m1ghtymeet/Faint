#pragma once

#include "AComponent.h"

#include "Event/Event.h"

#include <Types/Renderer/Model.h>

namespace Moon {
    class MeshRendererComponent : public AComponent {
    public:
        std::string ModelPath;

        MeshRendererComponent(Entity& p_owner);

        std::string GetName() override { return "MeshRenderer"; }

        /**
        * Defines the model to use
        * @param p_model
        */
        void SetModel(Model* p_model);

        /**
        * Returns the current model
        */
        Model* GetModel() const;
        
        void SetMesh(Mesh* p_mesh, int p_meshIndex = -1);
        void AddMesh(Mesh* p_mesh, int p_meshIndex = -1);
        void ClearMeshes();

        const std::vector<Mesh*>& GetMeshes() const;

        /**
        * Serialize the component
        */
        json Serialize();

        /**
        * Deserialize the component
        * @param j
        */
        void Deserialize(const json& j);

    private:
        Model* m_model = nullptr;
        std::vector<Mesh*> m_meshes;
        std::vector<int> m_meshIndices;

        Event<> m_modelChangedEvent;
	};
}