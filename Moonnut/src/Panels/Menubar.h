#pragma once
#include "../UI/Panels/PanelMenuBar.h"
#include "../UI/Panels/PanelWindow.h"
#include <functional>

struct MenuItem {
	std::string name;
	std::function<void()> action;
	std::string shortcut;
	bool checkable;
	bool checked;
};

struct AMenu {
	std::string name;
	std::vector<MenuItem> items;
};

class Menubar : public PanelMenuBar {
public:
	Menubar();

	void AddMenu(const std::string& p_name, const std::vector<MenuItem>& p_items);
	void AddMenuItem(const std::string& p_name, const MenuItem& p_item);
	void RegisterPanel(const std::string& p_name, PanelWindow& p_panel);

	std::vector<AMenu> GetMenus() const;

private:
	void DrawWidgets() override;

private:
	std::vector<AMenu> m_menus;
};