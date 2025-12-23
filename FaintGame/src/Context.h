#pragma once

#include <Core/Base.h>
#include <Core/Window.h>
#include <Scene/SceneManager.h>
#include <AssetManagment/ModelManager.h>
#include <AssetManagment/TextureManager.h>
#include <Scripting/ScriptEngine.h>
#include <Physics/PhysicsEngine.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>
#include <Core/Audio/Audio.h>

#include <filesystem>

class Context {
public:
	Context();

	~Context();

public:
	//const std::filesystem::path engineAssetsPath;
	//const std::filesystem::path projectAssetsPath;

	Moon::Settings::WindowSettings windowSettings;
	std::unique_ptr<Moon::Window> window;
	std::unique_ptr<Moon::Scripting::ScriptEngine> scriptEngine;
	std::unique_ptr<Moon::Physics::PhysicsEngine> physicsEngine;
	std::unique_ptr<Moon::Audio::AudioEngine> audioEngine;

	Moon::SceneManager sceneManager;

	Moon::AssetManagment::ModelManager modelManager;
	Moon::AssetManagment::TextureManager textureManager;
};