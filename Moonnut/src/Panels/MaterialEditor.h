#pragma once

#include "../UI/Panels/PanelWindow.h"
#include "../Misc/IconGenerator.h"

#include <Renderer/Types/Material.h>

namespace Moon {
	class Scene;
}

namespace Moon::Editor {
	class MaterialEditor : public PanelWindow {
	private:
		struct TextureSlot {
			std::string name;
			std::string displayName;
			glm::vec4 tintColor = { 1, 1, 1, 1 };
			bool hasToggle; // Whether there's a "hasTexture" toggle
		};
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		MaterialEditor(const std::string& p_title, bool p_opened = true, const PanelWindowSettings& p_windowSettings = PanelWindowSettings());

		void SetMaterial(Moon::Rendering::Material& p_material, bool p_liveEdit = false);
		void SetScene(Moon::Scene& p_scene);
		
		void Update(float deltaTime);

	public:
		bool materialChanged = false;
	private:
		void _Draw_Impl() override;
		void DrawTextureSlot(const TextureSlot& slot, float size);
		void DrawTextureGrid();
		void DrawAddPropertyPopup();

		// Tab Methods
		void DrawPropertiesTab();

		// UI Components
		void DrawPropertyControl(const std::string& name, Moon::Rendering::MaterialProperty& prop);
		void DrawVector2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f);
		void DrawVector3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f);
		void DrawColorControl(const std::string& label, glm::vec3& color);
		void DrawFloatControl(const std::string& label, float& value, float speed = 0.1f, float min = 0.0f, float max = 1.0f);

	private:
		Moon::Rendering::Material* m_material = nullptr;
		Moon::Scene* m_scene = nullptr;
		Moon::Shader* m_shader = nullptr;
		bool m_liveEdit = false;

		mutable bool m_propertiesDirty = true;
		mutable std::vector<std::string> m_cachedPropertyNames;

		// Presets
		static Moon::Assets::Texture* s_emptyTexture;

		std::vector<TextureSlot> m_textureSlots = {
			{ "u_AlbedoMap", "Albedo", {1, 1, 1, 1}, true },
			{ "u_NormalMap", "Normal", {0.5f, 0.5f, 1, 1}, true },
			{ "u_SpecularMap", "Specular", {0.3f, 0.3f, 0.3f, 1}, true },
			{ "u_AOMap", "Ambient Occlusion", {0, 0, 0, 1}, false },
			{ "u_EmissiveMap", "Emissive", {1, 1, 0, 1}, false }
		};
	};
}