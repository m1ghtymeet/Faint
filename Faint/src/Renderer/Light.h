#pragma once
#include "Math/Math.h"

namespace Faint {
	enum LightType { Point, Directional, Spot, Area };
	struct Light {
		float posX;
		float posY;
		float posZ;
		float colorR;
		float colorG;
		float colorB;
		float radius;
		float strength;
		int type;
	};
}