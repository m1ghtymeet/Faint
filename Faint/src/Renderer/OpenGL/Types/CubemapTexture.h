#pragma once
#include "Common/Types.h"
#include <glad/glad.h>
#include <string>

struct CubemapTexture {
	CubemapTexture() = default;
	void Load(std::string filepath);
	void Bake();
	void Bind(GLuint slot = 0);
	GLuint GetID() const;

	TextureData m_textureData[6];

private:
	GLuint m_ID;
	float width, height;
};