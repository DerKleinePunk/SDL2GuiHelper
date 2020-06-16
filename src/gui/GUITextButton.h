#pragma once
#include <SDL_ttf.h>
#include "GUIOnClickDecorator.h"

class GUIElement;


class GUITextButton : public GUIElement, public GUIOnClickDecorator
{
	std::string text_;
	RTTI_DERIVED(GUITextButton);
	el::Logger* logger_;
	bool centertext_;
	TTF_Font* font_;
	GUITexture* textureText_;
	int fontHeight_;
	bool smallFont_;
	int _corner;
	SDL_Color backgroundColorButton_;

    void GetFont();
    void RenderText();
public:
	GUITextButton(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);

	void Text(const std::string& text);
    void FontHeight(int fontHeight);
    
	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
	void ChangeBackColor(SDL_Color color);
	int GetCorner() const;
	void SetCorner(int corner);
	void Disable();
	void Enable();
};


