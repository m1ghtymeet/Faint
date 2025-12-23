#pragma once

#include "AComponent.h"
#include <Event/Event.h>

namespace Moon {
    class Entity;

	class ColliderComponent : public AComponent {
	public:
        ColliderComponent(Entity& p_owner);

        Event<ColliderComponent&> TriggerEnterEvent;
        Event<ColliderComponent&> TriggerStayEvent;
        Event<ColliderComponent&> TriggerExitEvent;

        Event<ColliderComponent&> CollisionEnterEvent;
        Event<ColliderComponent&> CollisionStayEvent;
        Event<ColliderComponent&> CollisionExitEvent;
	};
}