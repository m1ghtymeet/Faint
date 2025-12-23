#pragma once

#include "APanelTransformable.h"

namespace Moon::UI {
	/**
	* A simple panel that is transformable and without decorations (No background)
	*/
	class PanelWindow : public APanelTransformable {
	public:
		void _Base_Draw() override;

	private:
		int CollectFlags();
	};
}