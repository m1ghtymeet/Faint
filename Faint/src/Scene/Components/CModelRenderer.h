#pragma once

#include "AComponent.h"

#include "Event/Event.h"

#include "AssetManagment/AssetManager.h"

namespace Faint {
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
        //Event<> m_modelChanedEvent;
	};
}