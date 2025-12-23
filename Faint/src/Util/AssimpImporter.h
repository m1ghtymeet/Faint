#pragma once

#include <Types/Renderer/Model.h>
#include <Types/Renderer/SkinnedModel.h>
#include <string>
#include <vector>

namespace Moon::AssimpImporter {
	ModelData ImportFbx(const std::string& filepath);
	Moon::Rendering::SkinnedModelData ImportSkinnedFbx(const std::string& filepath);
}