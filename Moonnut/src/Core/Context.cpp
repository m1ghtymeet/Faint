#include "Context.h"
#include <Core/GlobalLocator.h>
#include <Debug/Log.h>
#include <AssetManagment/MeshManager.h>
#include <FileSystem/FileSystem.h>
#include "../Panels/FileSystemUI.h"

#include <iostream>
#include <array>

constexpr std::array<std::pair<int, int>, 13> kResolutions{
	std::make_pair(640, 360), // nHD
	std::make_pair(854, 480), // FWVGA
	std::make_pair(960, 540), // qHD
	std::make_pair(1024, 576), // WSVGA
	std::make_pair(1280, 720), // HD
	std::make_pair(1366, 768), // FWXGA
	std::make_pair(1600, 900), // HD+
	std::make_pair(1920, 1080), // Full HD
	std::make_pair(2560, 1440), // QHD
	std::make_pair(3200, 1800), // QHD+
	std::make_pair(3840, 2160), // 4K UHD
	std::make_pair(5120, 2880), // 5K
	std::make_pair(7680, 4320), // 8K UHD
};

Context::Context(const std::filesystem::path& m_projectFolder) :
		sceneManager((m_projectFolder / "Assets").string()) {

	windowSettings.title = "Moon";
	windowSettings.resizable = true;
	windowSettings.width = kResolutions.at(6).first;
	windowSettings.height = kResolutions.at(6).second;

	window = std::make_unique<Moon::Window>(windowSettings);
	window->InitImgui();

#if FT_DEBUG
	HZ_CORE_TRACE("Renderer: " + std::string((char*)glGetString(GL_RENDERER)));
	HZ_CORE_TRACE("Vendor: " + std::string((char*)glGetString(GL_VENDOR)));
	HZ_CORE_TRACE("Version: " + std::string((char*)glGetString(GL_VERSION)));
#endif

	physicsEngine = std::make_unique<Moon::Physics::PhysicsEngine>();
	scriptEngine = std::make_unique<Moon::Scripting::ScriptEngine>();
	audioEngine = std::make_unique<Moon::Audio::AudioEngine>();

	Moon::FileSystem::SetRootDirectory(Moon::FileSystem::GetParentPath(m_projectFolder.string()));
	FileBrowser::m_currentDirectory = Moon::FileSystem::RootDirectory;

	Moon::AssetManagment::MeshManager::Init();
	Moon::GlobalLocator::Provide<Moon::Window>(*window);
	Moon::GlobalLocator::Provide<Moon::SceneManager>(sceneManager);
	Moon::GlobalLocator::Provide<Moon::AssetManagment::ModelManager>(modelManager);
	Moon::GlobalLocator::Provide<Moon::AssetManagment::TextureManager>(textureManager);
	Moon::GlobalLocator::Provide<Moon::Physics::PhysicsEngine>(*physicsEngine);
	Moon::GlobalLocator::Provide<Moon::Scripting::ScriptEngine>(*scriptEngine);
	Moon::GlobalLocator::Provide<Moon::Audio::AudioEngine>(*audioEngine);
}

Context::~Context() {

}