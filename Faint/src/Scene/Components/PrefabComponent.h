#pragma once

#include "AssetManagment/Prefab.h"

namespace Faint {

	class PrefabComponent {
	public:
		Ref<Prefab> PrefabInstance;
		std::string Path;

		bool IsInitialized = false;

		void SetPrefab(Ref<Prefab> prefab) {
			PrefabInstance = prefab;
			Path = prefab->Path;
		}
	};
}