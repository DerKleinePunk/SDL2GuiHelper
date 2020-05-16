#pragma once
#include <SDL_ttf.h>

class GUIFontManager
{
	std::map<int, TTF_Font*> defaultFonts_;
	std::map<int, TTF_Font*> defaultSmallFonts_;

public:
	GUIFontManager();
	~GUIFontManager();
	static void FindFontHeight(const char* name, int height, int fontheight, TTF_Font*& font);
	TTF_Font* GetDefaultFont(int height);
	TTF_Font* GetDefaultSmallFont(int height);
};


