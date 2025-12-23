#include "CompositeRenderer.h"
#include <Debug/Assertion.h>
#include <ranges>

namespace Moon::Rendering {

	template<RenderFeatureType T, typename ...Args>
	inline T& CompositeRenderer::AddFeature(Args && ...p_args) {
		FT_CORE_ASSERT(!m_isDrawing, "You cannot add a render feature while drawing.");
		//FT_CORE_ASSERT(!, "You cannot add a render feature while drawing.");
		T* feature = new T(*this, std::forward<Args>(p_args)...);
		m_features.emplace(typeid(T), feature);
		return *feature;
	}

	template<RenderFeatureType T>
	inline bool CompositeRenderer::RemoveFeature() {
		FT_CORE_ASSERT(!m_isDrawing, "You cannot remove a render feature while drawing.");
		return m_features.erase(typeid(T)) > 0;
	}

	template<RenderFeatureType T>
	inline T& CompositeRenderer::GetFeature() const {
		auto it = m_features.find(typeid(T));
		FT_CORE_ASSERT(it != m_features.end(), "Couldn't find a render feature matching the given type T.");
		return *dynamic_cast<T*>(it->second.get());
	}

	template<RenderPassType T, typename ...Args>
	inline T& CompositeRenderer::AddPass(const std::string& p_name, uint32_t p_order, Args && ...p_args) {
		FT_CORE_ASSERT(!m_isDrawing, "You cannot add a render pass while drawing.");
		for (const auto& pass : m_passes | std::views::values) {
			FT_CORE_ASSERT(pass.first != p_name, "This pass name is already in use!");
		}
		T* pass = new T(*this, std::forward<Args>(p_args)...);
		m_passes.emplace(p_order, std::make_pair(p_name, std::unique_ptr<ARenderPass>(pass)));
		return *pass;
	}

	template<RenderPassType T>
	inline T& CompositeRenderer::GetPass(const std::string& p_name) const {
		for (const auto& pass : m_passes | std::views::values) {
			if (pass.first == p_name) {
				return dynamic_cast<T&>(*pass.second.get());
			}
		}
		FT_CORE_ASSERT(true, "Couldn't find a render pass matching the given type T.");
		return *static_cast<T*>(nullptr);
	}
}