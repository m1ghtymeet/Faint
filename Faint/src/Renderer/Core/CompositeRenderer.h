#pragma once

#include <typeindex>
#include <memory>
#include <string>
#include <map>
#include <future>

#include <Scene/Scene.h>

#include <Renderer/Shader.h>
#include <Renderer/Core/BaseRenderer.h>
#include <Renderer/Types/RenderPassType.h>
#include <Renderer/Types/RenderFeatureType.h>
#include "Renderer/Core/ARenderPass.h"
#include <Renderer/Features/ARenderFeature.h>
#include <Renderer/Data/Describable.h>
#include <Renderer/Shader.h>

#include <Types/Types.h>

namespace Moon::Rendering {

	class CompositeRenderer : public BaseRenderer, public Data::Describable {
	private:
		std::chrono::steady_clock::time_point m_lastFrameTime;
		double m_frameBudgetMs = 16.67; // 60 FPS

		std::future<void> m_asyncPrecompute;
	public:
		CompositeRenderer();

		virtual void BeginFrame(const Data::FrameDescriptor& p_frameData);

		virtual void DrawFrame() final;

		virtual void EndFrame() override;

		virtual void DrawEntity(PipelineState state, const RenderItem& renderItem);

		void SetFrameBudget(double ms);

		void PrecomputeNextFrame();

		template<RenderFeatureType T, typename... Args>
		T& AddFeature(Args&&... p_args);

		template<RenderFeatureType T>
		bool RemoveFeature();

		template<RenderFeatureType T>
		T& GetFeature() const;

		template<RenderPassType T, typename... Args>
		T& AddPass(const std::string& p_name, uint32_t p_order, Args&&... p_args);

		template<RenderPassType T>
		T& GetPass(const std::string& p_name) const;

		std::shared_ptr<Shader> CreateShader(const std::string& name, const std::vector<std::string>& paths);
		std::shared_ptr<Shader> GetShader(const std::string& name) const;
		void HotReloadShaders();

		std::shared_ptr<FrameBuffer> CreateFrameBuffer(const std::string& name, uint32_t width, uint32_t height);
		std::shared_ptr<FrameBuffer> GetFrameBuffer(const std::string& name) const;

	protected:
		std::unordered_map<std::type_index, std::unique_ptr<ARenderFeature>> m_features;
		std::multimap<uint32_t, std::pair<std::string, std::unique_ptr<ARenderPass>>> m_passes;
	
	private:
		std::optional<std::reference_wrapper<ARenderPass>> m_currentPass;
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
		std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> m_framebuffers;
	};
}

#include "CompositeRenderer.inl"