#pragma once

#include "AssetManagment/Serializable.h"
#include "Scene/Environment/ProcedularSky.h"
#include "Renderer/OpenGL/Types/CubemapTexture.h"

namespace Faint {

	enum class SkyType {
		ProceduralSky = 0,
		ClearColor = 1
	};

	class Environment : public ISerializable {
	public:
		Environment();
		~Environment() = default;

		SkyType CurrentSkyType;
		Ref<ProcedularSky> proceduralSkybox;
		Ref<CubemapTexture> cubemapTexture;
		Vec3 ColorClear;

		json Serialize() override;
		void Deserialize(const json& j) override;
	};
}