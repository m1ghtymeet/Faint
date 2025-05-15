#pragma once
#include "FileFormats.h"

namespace Faint::File {

	// Models
	void ExportModel(const ModelData& modelData);
	ModelData ImportModel(const std::string& filepath);
	ModelHeader ReadModelHeader(const std::string& filepath);
}
