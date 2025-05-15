#pragma once
#include "Core/Base.h"
#include "Scene/Components.h"
#include "Shader.h"

namespace Faint {

	struct TextParams {
		glm::vec4 Color{ 1.0f };
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
	};

	class Renderer2D {
	private:
		static unsigned int VAO;
		static unsigned int VBO;
		static Ref<Texture> g_fontTexture;
	public:
		static Shader UIShader;
		static Shader TextShader;
		static glm::mat4 Projection;

		static void Init();
		static void BeginDraw(glm::vec2 size);
		static void Update();
		static void Render();
		static void DrawRect();
		static void DrawRect(TransformComponent transform, glm::vec2 size, glm::vec4 color, float borderRadius = 0.0f);
		static void DrawRect(glm::vec2 position, glm::vec2 size, glm::vec4 color, float borderRadius = 0.0f);
		
		static void BlitText(const std::string& text, const std::string& fontName, int locX, int locY, float scale);
		static void DrawString(const std::string& string, Ref<FaintUI::Font> font, const glm::mat4& transform, TextParams text, int entityID);
		static void DrawString(const std::string& string, const glm::mat4& transform, const TextComponent& component, int entityID);
		
		static void EndDraw() {}
	};
}