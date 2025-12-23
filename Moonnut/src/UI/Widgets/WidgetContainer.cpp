#include "WidgetContainer.h"
#include <algorithm>
#include <span>
#include <ranges>

WidgetContainer::~WidgetContainer() {
	RemoveAllWidgets();
}

void WidgetContainer::RemoveWidget(AWidget& p_widget) {
	auto found = std::find_if(m_widgets.begin(), m_widgets.end(), [&p_widget](std::pair<AWidget*, EMemoryMode>& p_pair)
	{
		return p_pair.first == &p_widget;
	});

	if (found != m_widgets.end()) {
		//if (found->second == EMemoryMode::INTERNAL_MANAGMENT)
		//	delete found->first;

		m_widgets.erase(found);
	}
}

void WidgetContainer::RemoveAllWidgets() {
	std::for_each(m_widgets.begin(), m_widgets.end(), [](auto& pair)
		{
			//if (pair.second == EMemoryMode::INTERNAL_MANAGMENT)
			//	delete pair.first;
		});

	m_widgets.clear();
}

void WidgetContainer::ConsiderWidget(AWidget& p_widget, bool p_manageMemory)
{
}

void WidgetContainer::DrawWidgets() {

	std::vector<AWidget*> widgetsToDraw;
	widgetsToDraw.reserve(m_widgets.size());
	std::ranges::copy(m_widgets | std::views::keys, std::back_inserter(widgetsToDraw));
	if (m_reservedDrawOrder) [[unlikely]] {
		for (AWidget* widget : widgetsToDraw | std::views::reverse)
			widget->Draw();
	}
	else {
		for (AWidget* widget : widgetsToDraw)
			widget->Draw();
	}
}

std::vector<std::pair<AWidget*, EMemoryMode>>& WidgetContainer::GetWidgets() {
	return m_widgets;
}
