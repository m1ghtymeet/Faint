#pragma once

#include <concepts>

namespace Moon::Rendering {
	class ARenderFeature;

	template<typename T>
	concept RenderFeatureType = std::derived_from<T, ARenderFeature>;
}