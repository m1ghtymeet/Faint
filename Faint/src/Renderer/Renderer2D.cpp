#include "hzpch.h"
#include "Renderer2D.h"
#include "ShadersManager.h"
#include "UI/Font/Font.h"
#include "UI/TextBlitting/TextBlitter.h"
#include <glad/glad.h>

#include "Engine.h"
#include "Debug/Log.h"
#include "Core/Window.h"

#include <glm/gtx/matrix_decompose.hpp>

namespace Faint {

    Shader Renderer2D::UIShader;
    Shader Renderer2D::TextShader;

    unsigned int Renderer2D::VAO;
    unsigned int Renderer2D::VBO;
    Ref<Texture> Renderer2D::g_fontTexture;

    glm::mat4 Renderer2D::Projection;
	Ref<Window> window;

    void Renderer2D::Init() {

        HZ_CORE_INFO("Renderer 2D initializing");
        UIShader = ShaderManager::GetShader("ui");

        float quad_Vertices[] = {
            // positions      texture Coords
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f
        };

		window = Window::Get();

        // Setup plane VAO
        glGenVertexArrays(1, &Renderer2D::VAO);
        glGenBuffers(1, &Renderer2D::VBO);
        glBindVertexArray(Renderer2D::VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Renderer2D::VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_Vertices), &quad_Vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
        // Init text blitter
        FaintUI::TextBlitter::Init();
        g_fontTexture = CreateRef<Texture>("data/editor/fonts/StandardFont.png", false);
    }

    void Renderer2D::BeginDraw(glm::vec2 size) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        //Projection = glm::ortho(0.f, size.x, size.y, 0.f, -1.f, 1000.0f);
        UIShader.Bind();
        UIShader.SetMat4("projection", Projection);
    }

    void Renderer2D::Update() {

        int originX = 42;
        int originY = 42;
        float scale = 3.5f;
        std::string text = "Sirens blaring at us,\nbut he only sped up,\nmight leave in a [COL=0.9,0.1,0.1]bodybag[COL=1,1,1], \nnever in [COL=0,0.9,0]cuffs[COL=1,1,1].";
		///FaintUI::TextBlitter::BlitText(text, "StandardFont", originX, originY, Engine::GetCurrentWindow()->GetFrameBuffer()->GetSize().x, Engine::GetCurrentWindow()->GetFrameBuffer()->GetSize().y, scale);
        FaintUI::TextBlitter::Update();
    }

    void Renderer2D::Render() {

		//glViewport(0, 0, window->GetWidth(), window->GetHeight());
        glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Shader textShader = ShaderManager::GetShader("gl_text_blitter");
        textShader.Bind();
        // Render text
        FaintUI::FontMesh* fontMesh = FaintUI::TextBlitter::GetGLFontMesh("StandardFont");
        if (fontMesh) {
            g_fontTexture->Bind(0);
            glBindVertexArray(fontMesh->GetVAO());
            glDrawElements(GL_TRIANGLES, fontMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }
    }

    void Renderer2D::DrawRect() {
        glBindVertexArray(Renderer2D::VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void Renderer2D::DrawRect(TransformComponent transform, glm::vec2 size, glm::vec4 color, float borderRadius) {

        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform.GetGlobalMatrix(), scale, rotation, translation, skew, perspective);

        BeginDraw({ 1920.0f, 1080.0f });
        UIShader.SetMat4("model", transform.GetGlobalMatrix());
        UIShader.SetVec3("a_size", scale);


        glBindVertexArray(Renderer2D::VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

  //  void DrawString(const std::string& string, Ref<FaintUI::Font> font, const glm::mat4& transform, TextParams textParams, int entityID) {

		//const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		//const auto& metrics = fontGeometry.getMetrics();
		//Ref<Texture> fontAtlas = font->mAtlas;

		////s_Data.FontAtlasTexture = fontAtlas;

		//double x = 0.0;
		//double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		//double y = 0.0;

		//const float spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();

		//for (size_t i = 0; i < string.size(); i++)
		//{
		//	char character = string[i];
		//	if (character == '\r')
		//		continue;

		//	if (character == '\n')
		//	{
		//		x = 0;
		//		y -= fsScale * metrics.lineHeight + textParams.LineSpacing;
		//		continue;
		//	}

		//	if (character == ' ')
		//	{
		//		float advance = spaceGlyphAdvance;
		//		if (i < string.size() - 1)
		//		{
		//			char nextCharacter = string[i + 1];
		//			double dAdvance;
		//			fontGeometry.getAdvance(dAdvance, character, nextCharacter);
		//			advance = (float)dAdvance;
		//		}

		//		x += fsScale * advance + textParams.Kerning;
		//		continue;
		//	}

		//	if (character == '\t')
		//	{
		//		// NOTE(Yan): is this right?
		//		x += 4.0f * (fsScale * spaceGlyphAdvance + textParams.Kerning);
		//		continue;
		//	}

		//	auto glyph = fontGeometry.getGlyph(character);
		//	if (!glyph)
		//		glyph = fontGeometry.getGlyph('?');
		//	if (!glyph)
		//		return;

		//	double al, ab, ar, at;
		//	glyph->getQuadAtlasBounds(al, ab, ar, at);
		//	glm::vec2 texCoordMin((float)al, (float)ab);
		//	glm::vec2 texCoordMax((float)ar, (float)at);

		//	double pl, pb, pr, pt;
		//	glyph->getQuadPlaneBounds(pl, pb, pr, pt);
		//	glm::vec2 quadMin((float)pl, (float)pb);
		//	glm::vec2 quadMax((float)pr, (float)pt);

		//	quadMin *= fsScale, quadMax *= fsScale;
		//	quadMin += glm::vec2(x, y);
		//	quadMax += glm::vec2(x, y);

		//	float texelWidth = 1.0f / fontAtlas->GetWidth();
		//	float texelHeight = 1.0f / fontAtlas->GetHeight();
		//	texCoordMin *= glm::vec2(texelWidth, texelHeight);
		//	texCoordMax *= glm::vec2(texelWidth, texelHeight);

		//	// render here
		//	//s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
		//	//s_Data.TextVertexBufferPtr->Color = textParams.Color;
		//	//s_Data.TextVertexBufferPtr->TexCoord = texCoordMin;
		//	//s_Data.TextVertexBufferPtr->EntityID = entityID;
		//	//s_Data.TextVertexBufferPtr++;
		//	//
		//	//s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
		//	//s_Data.TextVertexBufferPtr->Color = textParams.Color;
		//	//s_Data.TextVertexBufferPtr->TexCoord = { texCoordMin.x, texCoordMax.y };
		//	//s_Data.TextVertexBufferPtr->EntityID = entityID;
		//	//s_Data.TextVertexBufferPtr++;
		//	//
		//	//s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
		//	//s_Data.TextVertexBufferPtr->Color = textParams.Color;
		//	//s_Data.TextVertexBufferPtr->TexCoord = texCoordMax;
		//	//s_Data.TextVertexBufferPtr->EntityID = entityID;
		//	//s_Data.TextVertexBufferPtr++;
		//	//
		//	//s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
		//	//s_Data.TextVertexBufferPtr->Color = textParams.Color;
		//	//s_Data.TextVertexBufferPtr->TexCoord = { texCoordMax.x, texCoordMin.y };
		//	//s_Data.TextVertexBufferPtr->EntityID = entityID;
		//	//s_Data.TextVertexBufferPtr++;
		//	//
		//	//s_Data.TextIndexCount += 6;
		//	//s_Data.Stats.QuadCount++;

		//	if (i < string.size() - 1)
		//	{
		//		double advance = glyph->getAdvance();
		//		char nextCharacter = string[i + 1];
		//		fontGeometry.getAdvance(advance, character, nextCharacter);

		//		x += fsScale * advance + textParams.Kerning;
		//	}
  //  }

void Renderer2D::BlitText(const std::string& text, const std::string& fontName, int locX, int locY, float scale)
{
	//FaintUI::TextBlitter::BlitText(text, fontName, locX, locY, Engine::GetCurrentWindow()->GetFrameBuffer()->GetSize().x, Engine::GetCurrentWindow()->GetFrameBuffer()->GetSize().y, scale);
}

void Renderer2D::DrawString(const std::string& string, const glm::mat4& transform, const TextComponent& component, int entityID)
    {
        //DrawString(string, component.FontAsset, transform, { component.Color, component.Kerning, component.LineSpacing }, entityID);
    }
}
