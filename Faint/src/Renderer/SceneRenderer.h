#pragma once

#include "Renderer/Core/CompositeRenderer.h"
#include <Renderer/Types/IMesh.h>
#include <Types/Types.h>

namespace Moon::Rendering {

	class SceneRenderer : public CompositeRenderer {
	public:
		enum class EOrderingMode {
			BACK_TO_FRONT,
			FRONT_TO_BACK,
		};

		template<EOrderingMode OrderingMode>
		struct DrawOrder {
			const int order;
			const float distance;
			/**
			* Determines the order of the drawables.
			* Current order is: order -> distance
			* @param p_other
			*/
			bool operator<(const DrawOrder& p_other) const {
				if (order == p_other.order) {
					if constexpr (OrderingMode == EOrderingMode::BACK_TO_FRONT) {
						return distance > p_other.distance;
					}
					else {
						return distance < p_other.distance;
					}
				}
				else {
					return order < p_other.order;
				}
			}
		};

		/**
		* Input data for the scene renderer.
		*/
		struct SceneDescriptor {
			Scene* scene;
		};

		/**
		* Result of the scene parsing, containing the renderitems to be rendered.
		*/
		struct SceneRenderItemsDescriptor {
			std::vector<RenderItem> renderItems;
		};

		/**
		* Filtered renderitems for the scene, categorized by their render pass, and sorted by their draw order.
		*/
		struct SceneFilteredRenderItemsDescriptor {
			std::multimap<DrawOrder<EOrderingMode::FRONT_TO_BACK>, RenderItem> opaques;
			std::multimap<DrawOrder<EOrderingMode::BACK_TO_FRONT>, RenderItem> transparents;
			std::multimap<DrawOrder<EOrderingMode::BACK_TO_FRONT>, RenderItem> ui;
		};
		
		SceneRenderer();

		virtual void BeginFrame(const Data::FrameDescriptor& p_frameData) override;

		SceneRenderItemsDescriptor ParseScene(const Scene& p_scene);

		SceneFilteredRenderItemsDescriptor FilterRenderItems(
			const SceneRenderItemsDescriptor& renderItems
		);

	private:
		std::vector<Light> m_lights;
	};
}