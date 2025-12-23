#include "Menubar.h"
#include <iostream>

#include <imgui.h>

Menubar::Menubar() {
}

void Menubar::AddMenu(const std::string& p_name, const std::vector<MenuItem>& p_items) {
    m_menus.push_back({ p_name, p_items });
}

void Menubar::AddMenuItem(const std::string& p_name, const MenuItem& p_item) {

    auto it = std::find_if(m_menus.begin(), m_menus.end(),
        [&](const AMenu& m) { return m.name == p_name; });

    if (it != m_menus.end()) {
        it->items.push_back(p_item);
    }
    else {
        m_menus.push_back({ p_name, {p_item} });
    }
}

void Menubar::RegisterPanel(const std::string& p_name, PanelWindow& p_panel) {
    AddMenuItem("Windows", {
        .name = p_name,
        .action = [&]() { p_panel.SetOpened(!p_panel.IsOpened()); },
        .checkable = p_panel.IsOpened()
    });
}

std::vector<AMenu> Menubar::GetMenus() const {
    return m_menus;
}

void Menubar::DrawWidgets() {
    for (auto& menu : m_menus) {
        if (ImGui::BeginMenu(menu.name.c_str())) {
            for (auto& item : menu.items) {
                if (ImGui::MenuItem(item.name.c_str(), item.shortcut.c_str(), item.checkable ? &item.checked : nullptr, enabled)) {
                    if (item.action) item.action();
                }
            }
            ImGui::EndMenu();
        }
    }
}
