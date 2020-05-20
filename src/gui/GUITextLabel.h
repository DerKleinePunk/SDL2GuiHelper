#pragma once

#include <SDL_ttf.h>
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"

struct GUIEvent;
class GUITexture;

class GUITextLabel : public GUIElement, public GUIOnClickDecorator
{
	std::string text_ ;
	bool showTime_;
	double angle_;
	int fontHeight_;
	bool smallFont_;
	TTF_Font* font_;
	GUITexture* textureText_;
	SDL_Color selectedColor_;
	SDL_Color selectedBackgroundColor_;
	SDL_Color normalBackgroundColor_;
	AnchorFlags textAnchor_;
	int _corner;
	RTTI_DERIVED(GUITextLabel);

	void GetFont();

public:
	GUITextLabel(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);
	GUITextLabel(const GUIPoint position, const GUISize size, const std::string& name, const SDL_Color background, const SDL_Color textcolor, const bool smallFont);
	GUITextLabel(const GUIPoint position, const GUISize size, const std::string& name, const SDL_Color background, const SDL_Color textcolor, const bool smallFont, const SDL_Color selectedColor, const SDL_Color selectedbackgroundColor);

	void Select() override;
	void Unselect() override;

	void Text(const std::string& text);
	void ShowTime(bool on);
	void Rotate(double angle);
	void FontHeight(int fontHeight);
	void TextAnchor(AnchorFlags flags);

	void Init() override;
	void RenderText();
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;

	int GetCorner() const;
	void SetCorner(int corner);
};


