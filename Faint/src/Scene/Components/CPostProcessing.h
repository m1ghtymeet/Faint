#pragma once

#include "AssetManagment/Serializable.h"
#include "AComponent.h"
#include <UI/Font.h>

namespace Moon {
	class Text2DComponent : public AComponent {
	public:
		Text2DComponent(Entity& p_owner);

		std::string GetName() override;

		// Text operations
		void SetText(const std::string& text);
		std::string GetText() const;

		// Color operations
		void SetColor(const glm::vec3& color);
		glm::vec3 GetColor() const;

		// Font operations
		void SetFont(const std::string& fontPath, float fontSize = 32.0f);
		std::string GetFontPath() const;
		float GetFontSize() const;
		std::shared_ptr<Font> GetFont() const;

		// Serialization
		json Serialize();
		void Deserialize(const json& j);

	private:
		void LoadFont();

	private:
		std::string m_text = "Hello World!";
		std::string m_fontPath = "data/editor/fonts/ClearSans-Regular.ttf";
		float m_fontSize = 32.0f;
		glm::vec3 m_color = { 1.0f, 1.0f, 1.0f };

		// Cache for performance
		std::shared_ptr<Font> m_font;
		bool m_fontDirty = true;
	};
}