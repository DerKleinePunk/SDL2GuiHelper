#pragma once

#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include <SDL_ttf.h>
#include <AppEvents.h>

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
	bool running_;
	int fullgrad_;
	RTTI_DERIVED(GUIMediaDisplay);

	void GetFont();
	void ApplicationEvent(const AppEvent event, void* data1, void* data2);
public:
	GUIMediaDisplay(const GUIPoint& position, const GUISize& size, const std::string& name, SDL_Color background, SDL_Color textcolor);
	GUIMediaDisplay(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor, bool smallFont);

	void FontHeight(int fontHeight);
	void TextAnchor(AnchorFlags flags);

	void Init() override;
	void RenderText();
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;

	void ButtonDownUpdate(Uint8 button, const GUIPoint& point) override;
	void ButtonUpUpdate(Uint8 button, const GUIPoint& point) override;
    void ButtonMoveUpdate(Uint8 button, const GUIPoint& point) override;
};


