#pragma once

#include <unordered_set>
#include <typeindex>

#include <Renderer/Data/FrameDescriptor.h>
#include <Renderer/Types/RenderPassType.h>

namespace Moon::Rendering {
	
	class CompositeRenderer;

	enum class FeatureExecutionPolicy {
		DEFAULT,
		WHITELIST_ONLY,
		ALWAYS,
		NEVER,
		FRAME_EVENTS_ONLY
	};

	class ARenderFeature {
	public:
		virtual ~ARenderFeature() = default;

		bool IsEnabled() const;

		ARenderFeature& Include(std::type_index p_type);

		template<RenderPassType T>
		ARenderFeature& Include() { return Include(typeid(T)); }

	protected:
		ARenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy);

		virtual void OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor);

		virtual void OnEndFrame();

		virtual void OnBeforeDraw();

	protected:
		CompositeRenderer& m_renderer;

	private:
		std::unordered_set<std::type_index> m_blacklist;
		std::unordered_set<std::type_index> m_whitelist;

		friend class CompositeRenderer;
	};
}