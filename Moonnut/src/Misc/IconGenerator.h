#pragma once
#include <Renderer/Types/Material.h>
#include <Types/Renderer/Model.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>

class IconGenerator {
public:
	IconGenerator();
	void Generate(Moon::Rendering::Material& material);
	void Generate(Moon::Model& model);
	Moon::Rendering::Texture* GenerateAndReturn(Moon::Rendering::Material& material);
	Moon::Rendering::Texture* GenerateAndReturn(Moon::Model& model);

	Moon::FrameBuffer m_previewFrameBuffer;
private:
	std::shared_ptr<Moon::Shader> m_shader;
	
};