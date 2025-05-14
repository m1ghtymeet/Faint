#include "ScriptingSystem.h"
#include "Scene/Components/NetScriptComponent.h"
#include "Scene/Components/WrenScriptComponent.h"
#include "Scene/Components/LuaScriptComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"

#include "Scripting/ScriptingEngineNet.h"
#include "Scripting/ScriptingEngine.h"
#include "Scripting/ScriptEngine.h"
#include "Physics/Physics.h"
//#include <UI/Parsers/CanvasParser.h>

namespace Faint
{
	ScriptingSystem::ScriptingSystem(Scene* scene)
	{
		m_scene = scene;
	}

	bool ScriptingSystem::Init()
	{
		/* Main Scripting*/
		ScriptEngine::Init();
		HZ_CORE_INFO("Initializing Lua Scripting");

		auto luaEntities = m_scene->Reg().view<LuaScriptComponent>();
		for (auto& e : luaEntities) {
			LuaScriptComponent& script = luaEntities.get<LuaScriptComponent>(e);
			if (script.path != "") {
				script.env = sol::environment(*ScriptEngine::luaState, sol::create, ScriptEngine::luaState->globals());
				ScriptEngine::luaState->script_file("data/editor/scripts/Engine.lua", script.env);
				ScriptEngine::luaState->script_file("data/editor/scripts/Class.lua", script.env);
				ScriptEngine::luaState->script_file("data/editor/scripts/Input.lua", script.env);
				std::cout << "Loaded Script: " << script.path << "\n";

				auto result = ScriptEngine::luaState->script_file(script.path, script.env);
				sol::table scriptClass = ScriptEngine::luaState->require_file(FileSystem::GetFileNameFromPath(script.path), script.path);
				//script.instance = result;
				script.instance = scriptClass["new"](scriptClass);

				script.instance["entity_id"] = (int)e;
				script.instance["entity_name"] = Engine::GetCurrentScene()->GetEntityByID((int)e).GetComponent<NameComponent>().name;
				if (script.instance["Start"].valid())
					script.instance["Start"](script.instance);
			}
			else {
				std::cout << "Script File was not found!\n";
			}

		}

		/* -------------- */
		ScriptingEngine::Init();
		HZ_CORE_INFO("Initializing ScriptingSystem");

		preInitDelegate.Broadcast();

		auto wrenEntities = m_scene->Reg().view<WrenScriptComponent>();
		for (auto& e : wrenEntities)
		{
			WrenScriptComponent& wren = wrenEntities.get<WrenScriptComponent>(e);

			if (!wren.mWrenScript)
				continue;

			wren.mWrenScript->Build(wren.mModule, true);

			if (!wren.mWrenScript->HasCompiledSuccesfully()) {
				HZ_CORE_ERROR("Failed to compile Wren script: {0}");
				return false;
			}

			wren.mWrenScript->SetScriptableEntityID((int)e);
			wren.mWrenScript->CallInit();
		}

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		scriptingEngineNet.Uninitialize();
		scriptingEngineNet.Initialize();
		scriptingEngineNet.LoadProjectAssembly(Engine::GetProject());

		auto netEntities = m_scene->Reg().view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_scene };

			// Creates an instance of the entity script in C#
			scriptingEngineNet.RegisterEntityScript(entity);

			netScriptComponent.Initialized = true;
		}

		// Instantiate UI widgets
		//for (auto& uiWidget : CanvasParser::Get().GetAllCustomWidgetInstance())
		//{
		//	scriptingEngineNet.RegisterCustomWidgetInstance(uiWidget.first.first, uiWidget.first.second, uiWidget.second);
		//}

		// Call OnInit on entity script instances
		for (auto& e : netEntities)
		{
			auto entity = Entity{ e, m_scene };

			if (entity.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity))
			{
				// We can now call on init on it.
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
				scriptInstance.InvokeMethod("OnStart");
			}
		}

		// Call OnInit on UI widgets
		//for (auto& widget : CanvasParser::Get().GetAllCustomWidgetInstance())
		//{
		//	const UUID& canvasInstanceUUID = widget.first.first;
		//	const UUID& widgetInstanceUUID = widget.first.second;
		//	if (scriptingEngineNet.HasCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID))
		//	{
		//		auto widgetInstance = scriptingEngineNet.GetCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID);
		//		widgetInstance.InvokeMethod("OnInit");
		//	}
		//}

		postInitDelegate.Broadcast();

		return true;
	}

	void ScriptingSystem::Update(Time ts)
	{
		if (!Engine::IsPlayMode())
			return;

		m_scene->Reg().view<LuaScriptComponent>().each([](auto entity, LuaScriptComponent& script) {
			if (script.instance["Update"].valid())
				script.instance["Update"](script.instance);
		});

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_scene->Reg().view<NetScriptComponent>();

		// Instance all the new entities
		std::vector<Entity> entityJustInstanced;
		for (auto& e : netEntities)
		{
			auto& netScriptComponent = netEntities.get<NetScriptComponent>(e);
			if (!netScriptComponent.Initialized)
			{
				if (netScriptComponent.ScriptPath.empty())
					continue;

				auto entity = Entity{ e, m_scene };

				// Creates an instance of the entity script in C#
				scriptingEngineNet.RegisterEntityScript(entity);

				netScriptComponent.Initialized = true;
				entityJustInstanced.push_back(entity);
			}
		}

		// Call init on the newly created instance
		for (auto& e : entityJustInstanced)
		{
			auto& netScriptComponent = e.GetComponent<NetScriptComponent>();
			if (e.IsValid() && scriptingEngineNet.HasEntityScriptInstance(e))
			{
				// We can now call on init on it.
				auto scriptInstance = scriptingEngineNet.GetEntityScript(e);
				scriptInstance.InvokeMethod("OnStart");
			}
		}

		// Then call update on all the other scripts
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnUpdate", ts.GetDeltaTime());
		}

		//for (auto& widget : CanvasParser::Get().GetAllCustomWidgetInstance())
		//{
		//	const UUID& canvasInstanceUUID = widget.first.first;
		//	const UUID& widgetInstanceUUID = widget.first.second;
		//	if (scriptingEngineNet.HasCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID))
		//	{
		//		auto widgetInstance = scriptingEngineNet.GetCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID);
		//		widgetInstance.InvokeMethod("OnTick", ts.GetSeconds());
		//	}
		//}

		DispatchPhysicCallbacks();
	}

	void ScriptingSystem::FixedUpdate(Time ts)
	{
		if (!Engine::IsPlayMode())
			return;

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_scene->Reg().view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnFixedUpdate", ts.GetDeltaTime());
		}
	}

	void ScriptingSystem::Exit()
	{
		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_scene->Reg().view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_scene };
			if (entity.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity))
			{
				HZ_CORE_TRACE(entity.GetComponent<NameComponent>().name);
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
				scriptInstance.InvokeMethod("OnDestroy");
			}
		}

		ScriptingEngineNet::Get().Uninitialize();
	}

	void ScriptingSystem::InitializeNewScripts()
	{
		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_scene->Reg().view<NetScriptComponent>();

		// Instance all the new entities
		std::vector<Entity> entityJustInstanced;
		for (auto& e : netEntities)
		{
			auto& netScriptComponent = netEntities.get<NetScriptComponent>(e);
			if (!netScriptComponent.Initialized)
			{
				if (netScriptComponent.ScriptPath.empty())
					continue;

				auto entity = Entity{ e, m_scene };

				// Creates an instance of the entity script in C#
				scriptingEngineNet.RegisterEntityScript(entity);

				netScriptComponent.Initialized = true;
				entityJustInstanced.push_back(entity);
			}
		}
	}

	void ScriptingSystem::DispatchPhysicCallbacks()
	{
		auto& scriptingEngineNet = ScriptingEngineNet::Get();

		//auto& physicsManager = PhysicsManager::Get();
		//const auto collisions = physicsManager.GetCollisions();
		//for (const auto& col : collisions)
		//{
		//	// Calling both collidee(?)
		//	Entity entity1 = { (entt::entity)col.Entity1, m_scene };
		//	if (entity1.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity1))
		//	{
		//		auto scriptInstance = scriptingEngineNet.GetEntityScript(entity1);
		//		scriptInstance.InvokeMethod("OnCollisionInternal", (int)col.Entity2);
		//	}
		//
		//	Entity entity2 = { (entt::entity)col.Entity2, m_scene };
		//	if (entity2.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity2))
		//	{
		//		auto scriptInstance = scriptingEngineNet.GetEntityScript(entity2);
		//		scriptInstance.InvokeMethod("OnCollisionInternal", (int)col.Entity1);
		//	}
		//}

		//physicsManager.GetWorld()->ClearCollisionData();
		Physics::ClearCollisionReports();
	}
}