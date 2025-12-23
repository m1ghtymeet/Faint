#include "UIBackEnd.h"
#include <Types/Types.h>
#include <Debug/Log.h>
#include <iostream>

namespace Moon::UIBackEnd {
    Rendering::OpenGLMesh2D g_uiMesh;
    std::vector<Rendering::Vertex2D> g_vertices;
    std::vector<uint32_t> g_indices;
    std::vector<UIRenderItem> g_renderItems;
}

void Moon::UIBackEnd::Init() {
	g_uiMesh.Create();
}

void Moon::UIBackEnd::Update() {
    g_uiMesh.UpdateVertexBuffer(g_vertices, g_indices);
    g_vertices.clear();
    g_indices.clear();
}

void Moon::UIBackEnd::EndFrame() {
    g_renderItems.clear();
}

void Moon::UIBackEnd::BlitText(const std::string& text, std::shared_ptr<Font> font, float fontSize, const glm::vec2& pos, const glm::vec3& color) {
	//if (!font) HZ_CORE_ERROR("Current font is null!"); return;
	if (font->GetAtlasTextureID() == 0) { HZ_CORE_ERROR("Invalid texture!"); return; }
	MeshData2D meshData;
	float x = pos.x;
	float y = pos.y;
	float scale = fontSize / font->GetPixelSize(); // Adjust scale based on atlas pixel size
	for (size_t i = 0; i < text.length(); ++i) {
		char c = text[i];
		const Glyph& g = font->GetGlyph(c);
		// Handle newlines
		if (c == '\n') {
			x = pos.x;
			y += g.advance * scale; // Use advance for line height, or a fixed value like 1.2 * fontSize
			continue;
		}
		if (g.size.x == 0 || g.size.y == 0) {
			x += g.advance * scale;
			continue;
		}
		float xpos = x + g.bearing.x * scale;
		float ypos = y - (g.size.y - g.bearing.y) * scale; // Adjust ypos to baseline
		float w = g.size.x * scale;
		float h = g.size.y * scale;
		// Calculate UVs from atlas (normalized 0-1)
		glm::vec2 uvMin = g.atlasPos;
		glm::vec2 uvMax = g.atlasPos + g.atlasSize;
		// Add 4 vertices for the quad (in clockwise or counter-clockwise order)
		uint32_t baseVertex = static_cast<uint32_t>(meshData.vertices.size());
		meshData.vertices.push_back({ {xpos, ypos + h}, {uvMin.x, uvMin.y}, {color, 1.0f} }); // Bottom-left
		meshData.vertices.push_back({ {xpos + w, ypos + h}, {uvMax.x, uvMin.y}, {color, 1.0f} }); // Bottom-right
		meshData.vertices.push_back({ {xpos + w, ypos}, {uvMax.x, uvMax.y}, {color, 1.0f} }); // Top-right
		meshData.vertices.push_back({ {xpos, ypos}, {uvMin.x, uvMax.y}, {color, 1.0f} }); // Top-left
		// Add indices for two triangles
		meshData.indices.push_back(baseVertex + 0);
		meshData.indices.push_back(baseVertex + 1);
		meshData.indices.push_back(baseVertex + 2);
		meshData.indices.push_back(baseVertex + 0);
		meshData.indices.push_back(baseVertex + 2);
		meshData.indices.push_back(baseVertex + 3);
		x += g.advance * scale;
	}
	// Now, instead of pushing to instances and flushing, store in UIBackEnd
	if (!meshData.vertices.empty()) {
		// Append to global vertices and indices in UIBackEnd
		uint32_t baseVertex = static_cast<uint32_t>(g_vertices.size());
		uint32_t baseIndex = static_cast<uint32_t>(g_indices.size());
		g_vertices.insert(g_vertices.end(), meshData.vertices.begin(), meshData.vertices.end());
		for (auto& idx : meshData.indices) {
			g_indices.push_back(idx + baseVertex);
		}
		// Create a UIRenderItem for this text batch
		UIBackEnd::UIRenderItem item;
		item.baseVertex = baseVertex;
		item.baseIndex = baseIndex;
		item.indexCount = static_cast<int>(meshData.indices.size());
		item.textureIndex = font->GetAtlasTextureID(); // Use atlas texture ID
		item.filter = 0; // Linear filtering for text
		// Optional: Set clipping if needed, e.g., based on some UI context
		// item.clipMinX = ...;
		g_renderItems.push_back(item);
	}
	HZ_CORE_INFO("BlitText: Processed {} chars, generated {} vertices for text '{}'", text.length(), meshData.vertices.size(), text);
}

Moon::Rendering::OpenGLMesh2D& Moon::UIBackEnd::GetUIMesh() {
    return g_uiMesh;
}

std::vector<Moon::UIBackEnd::UIRenderItem>& Moon::UIBackEnd::GetRenderItems() {
    return g_renderItems;
}

std::vector<Moon::Rendering::Vertex2D>& Moon::UIBackEnd::GetVertices() {
    return g_vertices;
}

std::vector<uint32_t>& Moon::UIBackEnd::GetIndices() {
    return g_indices;
}
