#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "Font.h"
#include <Renderer/OpenGL/GL_Mesh2D.h>

namespace Moon::UIBackEnd {

	struct UIRenderItem {
        int baseVertex = 0;
        int baseIndex = 0;
        int indexCount = 0;
        int textureIndex = 0;
        int filter = 0; // 0 for linear, 1 for nearest
        int clipMinX = -1;
        int clipMinY = -1;
        int clipMaxX = -1;
        int clipMaxY = -1;
	};

	void Init();
	void Update();
	void EndFrame();

    void BlitText(const std::string& text, std::shared_ptr<Font> font, float fontSize, const glm::vec2& pos, const glm::vec3& color);

    Rendering::OpenGLMesh2D& GetUIMesh();
    std::vector<UIRenderItem>& GetRenderItems();
    std::vector<Rendering::Vertex2D>& GetVertices();
    std::vector<uint32_t>& GetIndices();
}