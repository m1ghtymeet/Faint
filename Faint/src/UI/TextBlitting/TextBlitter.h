#pragma once
#include <string>
#include "FontMesh.h"

namespace FaintUI {
	
	namespace TextBlitter {

		void Init();
		void BlitText(const std::string& text,
			const std::string fontName,
			int locationX,
			int locationY,
			int viewportwidth,
			int viewportHeight,
			float scale);
		void Update();
		FontMesh* GetGLFontMesh(const std::string& name);
	}
}