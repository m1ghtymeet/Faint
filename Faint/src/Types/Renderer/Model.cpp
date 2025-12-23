#include "Model.h"
#include <Renderer/OpenGL/GL_BackEnd.h>
#include <glad/glad.h>

Moon::Model::~Model() {
    
}

void Moon::Model::AddMeshIndex(uint32_t index) {
	m_meshIndices.push_back(index);
}

const size_t Moon::Model::GetMeshCount() const {
    return m_meshIndices.size();
}

const std::vector<uint32_t>& Moon::Model::GetMeshIndices() const {
    return m_meshIndices;
}

std::vector<std::string>& Moon::Model::GetMaterialNames() {
    return m_modelData.materialNames;
}
