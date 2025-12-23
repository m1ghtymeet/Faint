#pragma once

#include <AssetManagment/Serializable.h>
#include "AComponent.h"
#include <Types/Renderer/Texture.h>

namespace Moon::UI {
	class WidgetComponent : public AComponent {
	public:
		WidgetComponent(Entity& p_owner);

		std::string GetName() override;

		void SetTexture(Assets::Texture* texture);
		Assets::Texture* GetTexture() const;

		void SetColor(const glm::vec3& color);
		const glm::vec3& GetColor() const;

		json Serialize();

		void Deserialize(const json& j);

	private:
		Assets::Texture* m_texture = nullptr;
		glm::vec3 m_color = glm::vec3(1.0f);
	};
}