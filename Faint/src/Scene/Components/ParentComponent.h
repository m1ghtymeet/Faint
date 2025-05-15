#pragma once
#include "../Entity.h"
#include "Component.h"

namespace Faint {

	class ParentComponent : public Component {
	public:
		int ParentID;
		Entity Parent;
		bool HasParent = false;
		std::vector<Entity> Children = std::vector<Entity>();

		ParentComponent() = default;

		bool RemoveChildren(Entity ent) {
			for (int i = 0; i < Children.size(); i++) {
				if (Children[i].GetHandle() == ent.GetHandle()) {
					Children.erase(Children.begin() + i);
					return true;
				}
			}
			return false;
		}

		json Serialize() {
			BEGIN_SERIALIZE();
			END_SERIALIZE();
		}
		bool Deserialize(const json& str) {

			return true;
		}
	};
}