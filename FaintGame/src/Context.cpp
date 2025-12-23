#include "Context.h"
#include <Core/GlobalLocator.h>

Context::Context() :
	//engineAssetsPath(std::filesystem::current_path() / "Data" / "Engine"),
	//projectAssetsPath(std::filesystem::current_path() / "User" / "Assets")
	sceneManager(/*m_projectFolder /*/"D:/A Moon Projects/Example/Assets")
	{
	
	//framebuffer = std::make_unique<Moon::Rendering::OpenGLFramebuffer>("Main");

	windowSettings.title = "Moon";
	windowSettings.resizable = false;
	windowSettings.width = 1600;
	windowSettings.height = 900;

	window = std::make_unique<Moon::Window>(windowSettings);

	physicsEngine = std::make_unique<Moon::Physics::PhysicsEngine>();
	scriptEngine = std::make_unique<Moon::Scripting::ScriptEngine>();
	audioEngine = std::make_unique<Moon::Audio::AudioEngine>();

	Moon::GlobalLocator::Provide<Moon::SceneManager>(sceneManager);
	Moon::GlobalLocator::Provide<Moon::AssetManagment::ModelManager>(modelManager);
	Moon::GlobalLocator::Provide<Moon::AssetManagment::TextureManager>(textureManager);
	Moon::GlobalLocator::Provide<Moon::Physics::PhysicsEngine>(*physicsEngine);
	Moon::GlobalLocator::Provide<Moon::Scripting::ScriptEngine>(*scriptEngine);
	Moon::GlobalLocator::Provide<Moon::Audio::AudioEngine>(*audioEngine);
}

Context::~Context() {

}