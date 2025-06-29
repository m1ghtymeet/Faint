#pragma once
#include "AComponent.h"
#include "AssetManagment/Prefab.h"

namespace Faint {

	class PrefabComponent : public AComponent {
	public:
		PrefabComponent(Entity& p_owner) : AComponent(p_owner) {}
		Ref<Prefab> PrefabInstance;
		std::string Path;

		std::string GetName() override { return "Prefab"; }

		bool IsInitialized = false;

		void SetPrefab(Ref<Prefab> prefab) {
			PrefabInstance = prefab;
			Path = prefab->Path;
		}
	};
}