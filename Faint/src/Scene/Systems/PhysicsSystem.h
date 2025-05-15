#pragma once
#include "Scene/Systems/System.h"

namespace Faint {

	class PhysicsSystem : public System {
	public:
		PhysicsSystem(Scene* scene);
		bool Init() override;
		void Draw() override {};
		void Update(Time ts) override;
		void FixedUpdate(Time ts) override;
		void Exit() override;
	
		void InitializeNewBodies();
	private:
		void InitializeShapes();
		void InitializeQuakeMap();
		void InitializeRigidbodies();
		void InitializeCharacterControllers();
	
		void ApplyForces();
	};
}