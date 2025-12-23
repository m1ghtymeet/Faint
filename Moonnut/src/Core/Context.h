#pragma once

#include <Core/Base.h>
#include <Core/Window.h>
#include <Scene/SceneManager.h>
#include <AssetManagment/ModelManager.h>
#include <AssetManagment/TextureManager.h>
#include <Scripting/ScriptEngine.h>
#include <Physics/PhysicsEngine.h>
#include <Core/Audio/Audio.h>

class Context {
public:
	Context(const std::filesystem::path& m_projectFolder);

	virtual ~Context();

public:
	Moon::Settings::WindowSettings windowSettings;
	std::unique_ptr<Moon::Window> window;
	std::unique_ptr<Moon::Scripting::ScriptEngine> scriptEngine;
	std::unique_ptr<Moon::Physics::PhysicsEngine> physicsEngine;
	std::unique_ptr<Moon::Audio::AudioEngine> audioEngine;

	Moon::SceneManager sceneManager;

	Moon::AssetManagment::ModelManager modelManager;
	Moon::AssetManagment::TextureManager textureManager;
};