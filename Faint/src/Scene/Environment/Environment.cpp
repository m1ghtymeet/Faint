#include "Environment.h"

namespace Faint {

	Environment::Environment()
	{
		CurrentSkyType = SkyType::ClearColor;

		proceduralSkybox = CreateRef<ProcedularSky>();
		cubemapTexture = CreateRef<CubemapTexture>();
		ColorClear = { 0.2f, 0.2f, 0.2f };
	}

	json Environment::Serialize()
	{
		BEGIN_SERIALIZE();
		END_SERIALIZE();
	}

	bool Environment::Deserialize(const json& j)
	{
		return true;
	}
}
