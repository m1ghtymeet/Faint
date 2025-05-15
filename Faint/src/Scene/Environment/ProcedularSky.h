#pragma once

#include "Core/Base.h"
#include "Math/Math.h"

namespace Faint {

	class Camera;
	class ProcedularSky {
	public:
		float SurfaceRadius = 6360e3f;
		float AtmosphereRadius = 6380e3f;
		Vec3 RayleighScattering = Vec3(58e-7f, 135e-7f, 331e-7f);
		Vec3 MieScattering = Vec3(2e-5f);
		float SunIntensity = 2.0f;

		Vec3 CenterPoint = Vec3(0.0f, -SurfaceRadius, 0.0f);
		Vec3 SunDirection = Vec3(0.20000f, 0.95917f, 0.20000f);
		unsigned int VAO;
		unsigned int VBO;
		ProcedularSky();
		void Draw(Matrix4 projection, Matrix4 view);

		Vec3 GetSunDirection();

		Ref<ProcedularSky> Copy();
	};
}