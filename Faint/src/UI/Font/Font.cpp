#include "Font.h"
#include "FileSystem/FileSystem.h"

FaintUI::Font::Font(const std::string& path)
{
	//mFreeTypeHandle = msdfgen::initializeFreetype();
	Load(path);
}

FaintUI::Font::~Font()
{
	//msdfgen::destroyFont(mFontHandle);
}

Ref<FaintUI::Font> FaintUI::Font::New(const std::string& path)
{
	return CreateRef<FaintUI::Font>(path);
}

bool FaintUI::Font::Load(const std::string& path)
{
	//bool exist = FileSystem::FileExists(path);
	//mFontHandle = msdfgen::loadFont(mFreeTypeHandle, path.c_str());
	//if (!mFontHandle)
	//	return false;
	//
	//msdfgen::FontMetrics matrics;
	//msdfgen::getFontMetrics(matrics, mFontHandle);
	//
	//LineHeight = (float)matrics.lineHeight;
	return true;
}
