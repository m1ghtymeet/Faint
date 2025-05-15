#pragma once

#include "Component.h"
#include "AssetManagment/Serializable.h"
#include "FileSystem/FileSystem.h"
#include "Scripting/WrenScript.h"

namespace Faint {

	class WrenScriptComponent : public Component {
	public:
		std::string Path;

		unsigned int mModule = 0;
		Ref<WrenScript> mWrenScript = nullptr;

		void LoadScript(const std::string& path)
		{
			Path = path;
			Ref<File> file = FileSystem::GetFile(path);

			if (!file)
			{
				HZ_CORE_ERROR("WrenScript * Failed to load script: {0}", path);
				return;
			}

			mWrenScript = CreateRef<WrenScript>(file, true);
			auto modules = mWrenScript->GetModules();
			if (modules.size() > 0)
				mModule = 0;
		}

		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Path);
			SERIALIZE_VAL(mModule);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {
			if (j.contains("Path")) {
				Path = j["Path"];
				if (!Path.empty())
					LoadScript(Path);
			}
			if (j.contains("mModule"))
				mModule = j["mModule"];
			
			return true;
		}
	};
}