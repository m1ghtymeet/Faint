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
	void Log(Coral::String string) {
		HZ_TRACE(".net - {0}", string.Data());
	}

	void LoadScene(Coral::String path) {
		if (!FileSystem::FileExists(path))
		{
			HZ_CORE_ERROR(".net/scene * Failed to load scene with path: {0}", std::string(path));
			return;
		}
	}

	void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
		SceneRenderer::DrawLine(start, end, color);
	}

	void EngineNetAPI::RegisterMethods() {
		RegisterMethod("Engine.LoadSceneIcall", &LoadScene);
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));

		// Debug renderer
		RegisterMethod("Debug.DrawLineIcall", &DrawLine);

		//RegisterMethod("Physic.RayCastIcall", &Raycast);
		//RegisterMethod("Physic.ShapeCastSphereIcall", &Raycast);
		//RegisterMethod("Physic.ShapeCastCapsuleIcall", &ShapeCastCapsule);
		//RegisterMethod("Physic.ShapeCastBoxIcall", &ShapeCastBox);
	}
}
