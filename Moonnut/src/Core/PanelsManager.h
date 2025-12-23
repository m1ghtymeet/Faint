#pragma once

#include <unordered_map>

#include "../UI/Canvas.h"
#include "../UI/Panels/APanel.h"
#include "../UI/Panels/PanelWindow.h"
#include "../Panels/Menubar.h"

class PanelsManager {
public:
	/**
	* Constructor
	* @param p_canvas
	*/
	PanelsManager(Canvas& p_canvas);

	template<typename T, typename... Args>
	void CreatePanel(const std::string& p_id, Args&&... p_args) {
		if constexpr (std::is_base_of<PanelWindow, T>::value) {
			m_panels.emplace(p_id, std::make_unique<T>(p_id, std::forward<Args>(p_args)...));
			T& instance = *static_cast<T*>(m_panels.at(p_id).get());
			GetPanelAs<Menubar>("Menubar").RegisterPanel(instance.name, instance);
		}
		else {
			m_panels.emplace(p_id, std::make_unique<T>(std::forward<Args>(p_args)...));
		}

		m_canvas.AddPanel(*m_panels.at(p_id));
	}

	template<typename T>
	T& GetPanelAs(const std::string& p_id) {
		return *static_cast<T*>(m_panels[p_id].get());
	}

private:
	std::unordered_map<std::string, std::unique_ptr<APanel>> m_panels;
	Canvas& m_canvas;
};