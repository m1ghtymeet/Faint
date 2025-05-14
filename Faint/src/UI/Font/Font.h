#pragma once

//#include "Core/Base.h"
//#include "Math/Math.h"
//#undef INFINITE
//#include <msdf-atlas-gen/msdf-atlas-gen.h>
//#include <ext/import-font.h>

#include "Renderer/Types/Texture.h"

#include <string>
#include <memory>

using namespace Faint;

namespace FaintUI {

	struct CharPos {
		double left;
		double right;
		double top;
		double bottom;
	};

	struct CharUV {
		glm::vec2 Pos;
		glm::vec2 Size;
	};

	class Char {
	private:
		unsigned int m_VAO;
		unsigned int m_VBO;

	public:
		unsigned int Unicode;
		float Advance;
		CharPos PlaneBounds;
		CharUV AtlasBounds;

		Char() {};
		Char(const unsigned int unicode, float advance, CharPos plane, CharUV atlas) {
			Unicode = unicode;
			Advance = advance;
			PlaneBounds = plane;
			AtlasBounds = atlas;
		}

		CharUV GetAtlasUV(const glm::vec2& atlasSize) {
			return AtlasBounds;
		}
	};

	class Font {
	public:
		Font(const std::string& path);
		Font() = default;
		~Font();

		float LineHeight = 0.0f;

		static Ref<Font> New(const std::string& path);

		//msdfgen::FontHandle* GetFontHandle() const { return mFontHandle; }

		void AddChar(const unsigned int unicode, float advance, CharPos plane, CharUV atlas)
		{
			this->Chars[unicode] = Char(unicode, advance, plane, atlas);
		}

		Char GetChar(unsigned int unicode)
		{
			if (Chars.find(unicode) != Chars.end())
				return Chars[unicode];
			return Char();
		}

		Ref<Texture> mAtlas;

	private:

		std::string mFilePath;
		//msdfgen::FontHandle* mFontHandle;
		//msdfgen::FreetypeHandle* mFreeTypeHandle;

		std::map<unsigned int, Char> Chars;

		bool Load(const std::string& path);
	};
}