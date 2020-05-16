#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIFontManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include "../exception/TTFException.h"
#include "GUIFontManager.h"
#include <SDL_ttf.h>

//https://github.com/grimfang4/SDL_FontCache
//https://fonts.google.com/specimen/Inconsolata

//#define DEFAULT_FONT "Ubuntu-Medium.ttf"
//#define DEFAULT_FONT "DesignSystemC700R.ttf"
//#define DEFAULT_FONT "Arkitech-Medium.ttf"
//#define DEFAULT_SMALL_FONT "DesignSystemC500R.otf"
#define DEFAULT_SMALL_FONT "Inconsolata-Regular.ttf"
#define DEFAULT_FONT "Inconsolata-Bold.ttf"
//#define DEFAULT_SMALL_FONT "Ubuntu-Medium.ttf"

GUIFontManager::GUIFontManager()
{
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);

	VLOG(3) << "ctor";

	if (TTF_Init() < -1) {
		throw TTFException("Init");
	}
}

GUIFontManager::~GUIFontManager() {
	VLOG(3) << "dtor";

	auto entry = defaultFonts_.begin();
	while (entry != defaultFonts_.end())
	{
		TTF_CloseFont(entry->second);
		++entry;
	}

	auto entrySmall = defaultSmallFonts_.begin();
	while (entrySmall != defaultSmallFonts_.end())
	{
		TTF_CloseFont(entrySmall->second);
		++entrySmall;
	}

	TTF_Quit();
}

void GUIFontManager::FindFontHeight(const char* name, const int height, int fontheight, TTF_Font*& font) {
	font = TTF_OpenFont(name, fontheight);
	if (font == nullptr) {
		throw TTFException("OpenFont");
	}
	auto fontheightReal = TTF_FontLineSkip(font);
	while(fontheightReal > height) {
		TTF_CloseFont(font);
		fontheight--;
		font = TTF_OpenFont(name, fontheight);
		fontheightReal = TTF_FontLineSkip(font);
	}
}

TTF_Font* GUIFontManager::GetDefaultFont(int height) {
	if (height <= 0) return nullptr;

	const auto fontEntry = defaultFonts_.find(height);
	if(fontEntry != defaultFonts_.end())
	{
		return fontEntry->second;
	}

	const auto fontheight = height - 2;
	TTF_Font* font;
	FindFontHeight(DEFAULT_FONT, height, fontheight, font);

	defaultFonts_.insert(std::make_pair(height, font));
	return font;
}

TTF_Font* GUIFontManager::GetDefaultSmallFont(int height) {
	if (height <= 0) return nullptr;

	const auto fontEntry = defaultSmallFonts_.find(height);
	if (fontEntry != defaultSmallFonts_.end())
	{
		return fontEntry->second;
	}

	const auto fontheight = height - 2;
	TTF_Font* font;
	FindFontHeight(DEFAULT_SMALL_FONT, height, fontheight, font);

	defaultSmallFonts_.insert(std::make_pair(height, font));
	return font;
}
