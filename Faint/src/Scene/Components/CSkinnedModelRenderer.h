#pragma once

#include "AComponent.h"
#include <Types/Renderer/SkinnedModel.h>
#include <AssetManagment/Serializable.h>

namespace Moon {
	class SkinnedMeshRendererComp : public AComponent {
	public:
		SkinnedMeshRendererComp(Entity& p_owner);
		
		std::string GetName() override;

		/**
		* Defines the model to use
		* @param p_model
		*/
		void SetModel(Rendering::SkinnedModel* p_model);

		/**
		* Returns the current model
		*/
		Rendering::SkinnedModel* GetModel() const;

		json Serialize();

		void Deserialize(const json& j);

	private:
		Rendering::SkinnedModel* m_model = nullptr;
	};
}