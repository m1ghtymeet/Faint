#pragma once
#include "Core/Base.h"
#include "Renderer/OpenGL/Types/Model.h"
#include "Renderer/OpenGL/Types/SkinnedModel.h"

namespace Faint {
	class ModelResourceInspector {
	private:
		Ref<Faint::Model> _model;
		Ref<Faint::SkinnedModel> _skinnedModel;
	public:
		ModelResourceInspector(Ref<Faint::Model> model);
		ModelResourceInspector(Ref<Faint::SkinnedModel> model);
		ModelResourceInspector() = default;
		~ModelResourceInspector() = default;

		void Draw();
	};
}