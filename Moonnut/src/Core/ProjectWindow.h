#pragma once

#include <Core/Window.h>
#include <AssetManagment/TextureManager.h>
#include <memory>
#include <optional>
#include <string>

class ProjectWindow {
public:
	ProjectWindow();

	~ProjectWindow();

	std::optional<std::string> Run();

private:
	std::unique_ptr<Moon::Window> m_window;

	Moon::AssetManagment::TextureManager textureManager;
};