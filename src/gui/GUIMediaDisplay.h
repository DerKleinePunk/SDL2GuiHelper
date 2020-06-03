#pragma once

#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include <SDL_ttf.h>

struct GUIEvent;
class GUITexture;
class IAudioManager;

class GUIMediaDisplay : public GUIElement, public GUIOnClickDecorator
{
	std::string text_ ;
	double angle_;
	int fontHeight_;
	bool smallFont_;
	TTF_Font* font_;
	GUITexture* textureText_;
	GUITexture* textureMediadata_;
	SDL_Color selectedColor_;
	AnchorFlags textAnchor_;
	IAudioManager* audioManager_;
	bool running_;
	int fullgrad_;
	RTTI_DERIVED(GUIMediaDisplay);

	void GetFont();

public:
	GUIMediaDisplay(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);
	GUIMediaDisplay(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor, bool smallFont);

	void FontHeight(int fontHeight);
	void TextAnchor(AnchorFlags flags);
	void StreamStatus(bool running);

	void Init() override;
	//Todo find better this is not good
	void SetAudioManager(IAudioManager* manager);
	void RenderText();
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
};


