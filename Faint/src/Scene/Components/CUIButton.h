#pragma once
#include "AComponent.h"
#include "AssetManagment/Serializable.h"

namespace Moon {
	class UIButtonComponent : public AComponent {
	public:
		UIButtonComponent(Entity& p_owner);

		std::string GetName() override;
		
		bool IsHovered() const;
		bool IsPressed() const;

		void SetNormalColor(const glm::vec3& color);
		void SetHoverColor(const glm::vec3& color);
		void SetPressedColor(const glm::vec3& color);

		const glm::vec3& GetNormalColor() const;
		const glm::vec3& GetHoverColor() const;
		const glm::vec3& GetPressedColor() const;

		void SetOnClick(const std::function<void()>& callback);
		void SetOnHover(const std::function<void()>& callback);

		void OnHoverEnter();
		void OnHoverExit();
		void OnPress();
		void OnRelease(bool inside);

		json Serialize();

		void Deserialize(const json& j);

	private:
		glm::vec3 m_normalColor = { 1.0f, 1.0f, 1.0f };
		glm::vec3 m_hoverColor = { 0.9f, 0.9f, 0.9f };
		glm::vec3 m_pressedColor = { 0.8f, 0.8f, 0.8f };

		bool m_hovered = false;
		bool m_pressed = false;

		std::function<void()> m_onClick;
		std::function<void()> m_onHover;
	};
}