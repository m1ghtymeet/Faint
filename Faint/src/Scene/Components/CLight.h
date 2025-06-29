#pragma once
#include "AComponent.h"
#include "AssetManagment/Serializable.h"

#include "Renderer/Light.h"

namespace Faint {
	class LightComponent : public AComponent {
	public:
		LightComponent(Entity& p_owner);
		~LightComponent();

		std::string GetName() override { return "Light"; }

		void Update(float deltaTime);
        void SetCastShadows(bool toggle);
        const glm::mat4 GetProjectionView();

		LightType Type = Point;
		Vec3 Color;
		bool CastShadows = true;

		// Point Light
		float Strength;
		float Radius;

		// Spot Light
		float Cutoff = 12.0f;
		float Outercutoff = 20.0f;

		// Diretional Light
		Vec3 Direction;
		bool SyncDirectionWithSky = false;

	private:
		glm::mat4 m_viewMatrix[6];

	public:
		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Type);
			SERIALIZE_VAL(Radius);
			SERIALIZE_VAL(Strength);
			SERIALIZE_VEC3(Color);
			SERIALIZE_VAL(SyncDirectionWithSky);
			SERIALIZE_VAL(CastShadows);
			SERIALIZE_VAL(Cutoff);
			SERIALIZE_VAL(Outercutoff);
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			if (j.contains("Type"))
				Type = (LightType)j["Type"];
			if (j.contains("Color"))
				Color = Vec3(j["Color"]["x"], j["Color"]["y"], j["Color"]["z"]);
			if (j.contains("Strength"))
				Strength = j["Strength"];
			if (j.contains("SyncDirectionWithSky"))
				SyncDirectionWithSky = j["SyncDirectionWithSky"];
			if (j.contains("CastShadows"))
                SetCastShadows(j["CastShadows"]);
			if (j.contains("Direction")) {
				float x = j["Direction"]["x"];
				float y = j["Direction"]["y"];
				float z = j["Direction"]["z"];
				this->Direction = Vec3(x, y, z);
			}
			return true;
		}
	};
}