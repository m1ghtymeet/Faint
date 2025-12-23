#pragma once

#include <concepts>

namespace Moon::Rendering {
	class ARenderPass;

	template<typename T>
	concept RenderPassType = std::derived_from<T, ARenderPass>;
}