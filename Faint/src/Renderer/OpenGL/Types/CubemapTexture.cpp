#include "CubemapTexture.h"
#include <stb_image.h>
#include <glad/glad.h>
#include <vector>
#include <iostream>

void CubemapTexture::Load(std::string filepath) {

    std::vector<std::string> filepaths;
    filepaths.push_back(filepath + "_Right.png");
    filepaths.push_back(filepath + "_Left.png");
    filepaths.push_back(filepath + "_Top.png");
    filepaths.push_back(filepath + "_Bottom.png");
    filepaths.push_back(filepath + "_Front.png");
    filepaths.push_back(filepath + "_Back.png");
    for (unsigned int i = 0; i < 6; i++) {
        stbi_set_flip_vertically_on_load(0);
        m_textureData[i].m_data = stbi_load(filepaths[i].c_str(), &m_textureData[i].m_width, &m_textureData[i].m_height, &m_textureData[i].m_numChannels, 0);
    }
}

void CubemapTexture::Bake() {
    width = m_textureData[0].m_width;
    height = m_textureData[0].m_height;
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
    for (unsigned int i = 0; i < 6; i++) {
        if (m_textureData[i].m_data) {
            GLint format = GL_RGB;
            if (m_textureData[i].m_numChannels == 4)
                format = GL_RGBA;
            if (m_textureData[i].m_numChannels == 1)
                format = GL_RED;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, m_textureData[i].m_data);
            stbi_image_free(m_textureData[i].m_data);
        }
    }
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void CubemapTexture::Bind(GLuint slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

GLuint CubemapTexture::GetID() const {
    return m_ID;
}
