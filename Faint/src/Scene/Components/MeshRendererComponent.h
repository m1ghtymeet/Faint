#pragma once

#include "Component.h"

#include "Core/Base.h"
#include "Core/String.h"

#include "AssetManagment/ModelLoader.h"
#include "AssetManagment/AssetManager.h"

namespace Faint {

	struct MeshRendererComponent : public Component {
    public:
		std::string ModelPath;
		Ref<Model> _Model;

		void LoadModel() {
			auto loader = ModelLoader();
            _Model = loader.LoadModel(ModelPath, true);
		}

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(const std::string filePath) {
			this->ModelPath = filePath;
		}

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(ModelPath);
            if (_Model)
            {
                SERIALIZE_OBJECT(_Model);
            }
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            ModelPath = j["ModelPath"];
            _Model = CreateRef<Model>();

            if (ModelPath.empty() || !String::EndsWith(ModelPath, ".mesh"))
            {
                if (j.contains("ModelResource"))
                {
                    auto& res = j["Model"];
                    _Model->Deserialize(res);
                }
            }
            else
            {
                _Model = AssetManager::LoadModel(ModelPath);
            }

            return true;
        }
	};
}