#pragma once

class Panel {
public:
	Panel() = default;

	virtual void OnImGuiRender() = 0;
};