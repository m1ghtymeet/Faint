#pragma once
#include "Core/Base.h"
#include <vector>

class ProjectSettingsCategory {
public:
	std::string name = "undefined";
	virtual void Draw() = 0;
};

class ProjectSettingsCategoryAudio : public ProjectSettingsCategory {
public:
	ProjectSettingsCategoryAudio() { name = "Audio"; }
	void Draw() override;
};

class ProjectSettingsCategoryGraphics : public ProjectSettingsCategory {
public:
	ProjectSettingsCategoryGraphics() { name = "Graphics"; }
	void Draw() override;
};

class ProjectSettingsCategoryInputManager : public ProjectSettingsCategory {
public:
	ProjectSettingsCategoryInputManager() { name = "Input Manager"; }
	void Draw() override;
};

class ProjectSettingsCategoryPhysics : public ProjectSettingsCategory {
public:
	ProjectSettingsCategoryPhysics() { name = "Physics"; }
	void Draw() override;
};

class ProjectSettingsPanel {
private:
	std::vector<Ref<ProjectSettingsCategory>> categories;
public:
	ProjectSettingsPanel();
	void Draw(bool visible = true);
};