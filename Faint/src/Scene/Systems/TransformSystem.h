#pragma once
#include "Scene/Systems/System.h"

namespace Faint {

	class Scene;
	class Entity;

	class TransformSystem : public System {
	public:
		TransformSystem(Scene* scene);
		bool Init() override;
		void Update(Time ts) override;
		void Draw() override {}
		void FixedUpdate(Time ts) override;
		void Exit() override;

	private:
		void UpdateTransform();
		void UpdateDirtyFlagRecursive(Entity& entity);
		void CalculateGlobalTransform(Entity& entity);
	};
}