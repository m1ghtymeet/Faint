#include "CPrefab.h"

Moon::PrefabComponent::PrefabComponent(Entity& p_owner) : AComponent(p_owner) {
}

std::string Moon::PrefabComponent::GetName() {
    return "Prefab";
}

json Moon::PrefabComponent::Serialize() {
    BEGIN_SERIALIZE();
    j["ID"] = std::to_string(id);
    j["filepath"] = filepath;
    j["overrides"] = overridenComponents;
    END_SERIALIZE();
}

void Moon::PrefabComponent::Deserialize(const json& j) {
    if (j.contains("ID"))
		id = std::stoi(j["ID"].get<std::string>());
	if (j.contains("filepath"))
		filepath = j["filepath"].get<std::string>();
	if (j.contains("overrides"))
		overridenComponents = j["overrides"];
}
