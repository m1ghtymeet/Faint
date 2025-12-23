#include "SkinnedModel.h"

void Moon::Rendering::SkinnedModel::AddMeshIndex(uint32_t index) {
	m_meshIndices.push_back(index);
}

std::vector<uint32_t>& Moon::Rendering::SkinnedModel::GetMeshIndices() {
	return m_meshIndices;
}
