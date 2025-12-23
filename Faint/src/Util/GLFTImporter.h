#pragma once

#include <Types/Renderer/Model.h>

namespace Moon::GLTFImporter {
	ModelData ImportGltf(const std::string& filepath);
}