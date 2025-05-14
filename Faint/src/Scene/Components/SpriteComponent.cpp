#include "SpriteComponent.h"
#include "FileSystem/FileSystem.h"
#include "math/Math.h"
#include "Common/Types.h"

namespace Faint {

	SpriteComponent::SpriteComponent()
		: Billboard(false), LockYRotation(false)
	{
	}

	void SpriteComponent::SetSprite(std::string path)
	{
		if (!FileSystem::FileExists(path)) {
			return;
		}

		SpritePath = path;

		std::vector<Vertex> quadVertices =
		{
			{ Vec3(-1.0f,  1.0f, 0.0f), Vec3(0, 0, 1), Vec2(0.0f, 1.0f), Vec3(1, 0, 0), /*Vec3(0, 1, 0) */ },
			{ Vec3(1.0f,  1.0f, 0.0f), Vec3(0, 0, 1), Vec2(1.0f, 1.0f), Vec3(1, 0, 0), /*Vec3(0, 1, 0)*/ },
			{ Vec3(-1.0f, -1.0f, 0.0f), Vec3(0, 0, 1), Vec2(0, 0), Vec3(1, 0, 0), /*Vec3(0, 1, 0) */ },

			{ Vec3(1.0f,  -1.0f, 0.0f), Vec3(0, 0, 1), Vec2(1.0f, 0.0f), Vec3(1, 0, 0), /*Vec3(0, 1, 0)*/ },
			{ Vec3(-1.0f, -1.0f, 0.0f), Vec3(0, 0, 1), Vec2(0.0f, 0.0f), Vec3(1, 0, 0), /*Vec3(0, 1, 0)*/ },
			{ Vec3(1.0f,   1.0f, 0.0f), Vec3(0, 0, 1), Vec2(1.0f, 1.0f), Vec3(1, 0, 0), /*Vec3(0, 1, 0)*/ }
		};

		SpriteMesh = CreateRef<Mesh>();
		SpriteMesh->AddSurface(quadVertices, { 0, 1, 2, 3, 4, 5 });
	}

	bool SpriteComponent::LoadSprite()
	{
		return true;
	}

	json SpriteComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Billboard);
		SERIALIZE_VAL(LockYRotation);
		SERIALIZE_VAL(PositionFacing);
		END_SERIALIZE();
	}

	bool SpriteComponent::Deserialize(const json& j)
	{
		if (j.contains("Billboard"))
			Billboard = j["Billboard"];
		if (j.contains("LockYRotation"))
			Billboard = j["LockYRotation"];
		if (j.contains("PositionFacing"))
			Billboard = j["PositionFacing"];
		return true;
	}
}
