#pragma once

#include <Types/Renderer/Model.h>
#include <Types/Renderer/SkinnedModel.h>

namespace Moon::Loaders {

	/**
	* Handle the Model creation and destruction
	*/
	class ModelLoader {
	public:
		ModelLoader() = delete;

		static Model* Create(const std::string& p_filepath, bool p_absolute = true, bool needToCache = false);

		static void Reload(Model& p_model, const std::string& p_filepath);

		static bool Destroy(Model*& p_modelInstance);
	};

	/**
	* Handle the Skinned Model creation and destruction
	*/
	class SkinnedModelLoader {
	public:
		SkinnedModelLoader() = delete;

		static Moon::Rendering::SkinnedModel* Create(const std::string& p_filepath, bool p_absolute = true, bool needToCache = false);

		static void Reload(Moon::Rendering::SkinnedModel& p_model, const std::string& p_filepath);

		static bool Destroy(Moon::Rendering::SkinnedModel*& p_modelInstance);
	};
}