#pragma once
#include <vector>
#include "AWidget.h"

enum class EMemoryMode {
	INTERNAL_MANAGMENT,
	EXTERNAL_MANAGMENT
};

class WidgetContainer {
public:
	WidgetContainer() = default;

	virtual ~WidgetContainer();

	void RemoveWidget(AWidget& p_widget);

	void RemoveAllWidgets();

	void ConsiderWidget(AWidget& p_widget, bool p_manageMemory = true);

	void DrawWidgets();

	template<typename T, typename... Args>
	T& CreateWidget(Args&&... p_args) {
		m_widgets.emplace_back(new T(p_args...), EMemoryMode::INTERNAL_MANAGMENT);
		T& instance = *reinterpret_cast<T*>(m_widgets.back().first);
		return instance;
	}

	std::vector<std::pair<AWidget*, EMemoryMode>>& GetWidgets();

protected:
	std::vector<std::pair<AWidget*, EMemoryMode>> m_widgets;
	bool m_reservedDrawOrder = false;
};