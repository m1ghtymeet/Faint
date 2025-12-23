#pragma once
#include <string>
#include "../Plugins/Pluginable.h"

class AWidget : public Pluginable {
public:
	AWidget();

	virtual void Draw();

	void Destroy();

	bool IsDestroyed() const;

protected:
	virtual ~AWidget() = default;
	virtual void _Draw_Impl() = 0;

public:
	std::string tooltip;
	bool enabled = true;
	bool disabled = false;
	bool lineBreak = true;

protected:
	std::string m_widgetID = "undefined";

private:
	static uint64_t __WIDGET_ID_INCREMENT;
};