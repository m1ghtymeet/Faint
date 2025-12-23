#include "CText2D.h"
#include <Debug/Log.h>
#include <UI/FontManager.h>

Moon::Text2DComponent::Text2DComponent(Entity& p_owner)
	: AComponent(p_owner)
{
	//LoadFont();
}

std::string Moon::Text2DComponent::GetName() {
	return "Text2D Component";
}

void Moon::Text2DComponent::SetText(const std::string& text) {
	if (m_text != text)
		m_text = text;
}

void Moon::Text2DComponent::SetColor(const glm::vec3& color) {
	m_color = color;
}

std::string Moon::Text2DComponent::GetText() const {
	return m_text;
}

glm::vec3 Moon::Text2DComponent::GetColor() const {
	return m_color;
}

void Moon::Text2DComponent::SetFont(const std::string& fontPath, float fontSize) {
	if (m_fontPath != fontPath || m_fontSize != fontSize) {
		m_fontPath = fontPath;
		m_fontSize = fontSize;
		m_fontDirty = true;
		LoadFont();
	}
}

std::string Moon::Text2DComponent::GetFontPath() const {
	return m_fontPath;
}

float Moon::Text2DComponent::GetFontSize() const {
	return m_fontSize;
}

std::shared_ptr<Font> Moon::Text2DComponent::GetFont() const {
	return m_font;
}

json Moon::Text2DComponent::Serialize() {
	BEGIN_SERIALIZE();
	j["text"] = m_text;
	j["fontPath"] = m_fontPath;
	j["fontSize"] = m_fontSize;
	j["color"] = { m_color.x,m_color.y, m_color.z };
	END_SERIALIZE();
}

void Moon::Text2DComponent::Deserialize(const json& j) {
	if (j.contains("text")) m_text = j["text"];
	if (j.contains("fontPath")) m_fontPath = j["fontPath"];
	if (j.contains("fontSize")) m_fontSize = j["fontSize"];
	if (j.contains("color")) {
		m_color = {
			j["color"][0],
			j["color"][1],
			j["color"][2]
		};
	}
	m_fontDirty = true;
	LoadFont();
}

void Moon::Text2DComponent::LoadFont() {
	if (m_fontDirty) {
		m_font = FontManager::LoadFont(m_fontPath, m_fontSize);
#ifdef FT_DEBUG
		HZ_CORE_INFO("=== FONT DEBUG ===");
		HZ_CORE_INFO("Path: {}", m_fontPath);
		HZ_CORE_INFO("Font ptr: {}", (void*)m_font.get());
		if (m_font) {
			HZ_CORE_INFO("Texture ID: {}", m_font->GetAtlasTextureID());
			if (m_font->GetAtlasTextureID() == 0) { HZ_CORE_ERROR("TEXTURE IS 0!"); }
			else HZ_CORE_INFO("TEXTURE OK!");
		}
#endif
		m_fontDirty = false;
		if (!m_font)
			HZ_CORE_ERROR("Failed to load font: {}", m_fontPath);
	}
}
