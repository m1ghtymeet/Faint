#pragma once
#include "Core/Base.h"
#include "Renderer/Types/Model.h"

namespace Faint {
	class ModelResourceInspector {
	private:
		Ref<Faint::Model> _model;
	public:
		ModelResourceInspector(Ref<Faint::Model> model);
		ModelResourceInspector() = default;
		~ModelResourceInspector() = default;

		void Draw();
	};
}