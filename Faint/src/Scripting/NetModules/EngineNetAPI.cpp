#include "EngineNetAPI.h"
#include <iostream>

#include "Engine.h"
#include "Scene/Scene.h"
#include "Renderer/SceneRenderer.h"
#include "Debug/Log.h"
#include "Math/Math.h"
#include "FileSystem/FileSystem.h"
#include "Physics/Physics.h"

#include <Coral/String.hpp>
#include <Coral/ManagedObject.hpp>
#include <Coral/Array.hpp>

namespace Faint {

	struct CapsuleInternal
	{
		float Radius;
		float Height;
	};

	struct BoxInternal
	{
		float x;
		float y;
		float z;
	};

	void Log(Coral::String string)
	{
		HZ_TRACE(".net - {0}", string.Data());
	}

	void LoadScene(Coral::String path)
	{
		if (!FileSystem::FileExists(path))
		{
			HZ_CORE_ERROR(".net/scene * Failed to load scene with path: {0}", std::string(path));
			return;
		}
	}

	void DrawShapeBox(Vec3 position, Quat rotation, Vec3 box, Vec4 color, float life, float width)
	{
		//Engine::GetCurrentScene()->m_sceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Box>(box.x, box.y, box.z), color, life, width);
	}
	void DrawShapeSphere(Vec3 position, Quat rotation, float radius, Vec4 color, float life, float width)
	{
		//Engine::GetCurrentScene()->m_sceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Sphere>(radius), color, life, width);
	}
	void DrawShapeCapsule(Vec3 position, Quat rotation, float radius, float height, Vec4 color, float life, float width)
	{
		//Engine::GetCurrentScene()->m_sceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Capsule>(radius, height), color, life, width);
	}

	Coral::Array<float> ConvertHitsToArray(const PhysXRayResult hit)
	{
		std::vector<float> results;
		//for (const auto& hit : hits)
		//{
		//	results.push_back(hit.ImpactPosition.x);
		//	results.push_back(hit.ImpactPosition.y);
		//	results.push_back(hit.ImpactPosition.z);
		//	results.push_back(hit.ImpactNormal.x);
		//	results.push_back(hit.ImpactNormal.y);
		//	results.push_back(hit.ImpactNormal.z);
		//	results.push_back(hit.Fraction);
		//	results.push_back(hit.Layer);
		//	results.push_back(hit.EntityID);
		//}
		return Coral::Array<float>::New(results);
	}

	Coral::Array<float> Raycast(float fromX, float fromY, float fromZ, float toX, float toY, float toZ)
	{
		const Vec3 from = { fromX, fromY, fromZ };
		const Vec3 to = { toX, toY, toZ };
		//auto hits = PhysicsManager::Get().Raycast(from, to);
		//return ConvertHitsToArray(hits);
		return Coral::Array<float>();
	}

	Coral::Array<float> ShapeCastCapsule(float fromX, float fromY, float fromZ, float toX, float toY, float toZ, CapsuleInternal capsuleInternal)
	{
		//auto capsule = CreateRef<Physics::Capsule>(capsuleInternal.Radius, capsuleInternal.Height);
		const Vec3 from = { fromX, fromY, fromZ };
		const Vec3 to = { toX, toY, toZ };
		//auto hits = PhysicsManager::Get().Shapecast(from, to, capsule);
		//return ConvertHitsToArray(hits);
		return Coral::Array<float>();
	}

	Coral::Array<float> ShapeCastBox(float fromX, float fromY, float fromZ, float toX, float toY, float toZ, BoxInternal boxInternal)
	{
		//auto capsule = CreateRef<Physics::Box>(boxInternal.x, boxInternal.y, boxInternal.z);
		const Vec3 from = { fromX, fromY, fromZ };
		const Vec3 to = { toX, toY, toZ };
		//auto hits = PhysicsManager::Get().Shapecast(from, to, capsule);
		//return ConvertHitsToArray(hits);
		return Coral::Array<float>();
	}

	void EngineNetAPI::RegisterMethods()
	{
		RegisterMethod("Engine.LoadSceneIcall", &LoadScene);
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));

		// Debug renderer
		RegisterMethod("Debug.DrawShapeBoxIcall", &DrawShapeBox);
		RegisterMethod("Debug.DrawShapeSphereIcall", &DrawShapeSphere);
		RegisterMethod("Debug.DrawShapeCapsuleIcall", &DrawShapeCapsule);

		RegisterMethod("Physic.RayCastIcall", &Raycast);
		//RegisterMethod("Physic.ShapeCastSphereIcall", &Raycast);
		RegisterMethod("Physic.ShapeCastCapsuleIcall", &ShapeCastCapsule);
		RegisterMethod("Physic.ShapeCastBoxIcall", &ShapeCastBox);
	}
}
