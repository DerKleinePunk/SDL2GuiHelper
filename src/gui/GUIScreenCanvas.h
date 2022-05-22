#pragma once
#include "GUIElement.h"

class GUIScreenCanvas : public GUIElement
{
	friend class GUIScreen;
	std::string _backgroundImage;
	GUITexture* imageTexture_;
	std::string _text;
	RTTI_DERIVED(GUIScreen);
	TTF_Font* _font;
	GUITexture* _textureText;
	int _fontHeight;
	void GetFont();
	void RenderText();
protected:
	explicit GUIScreenCanvas(GUISize size, const std::string& backgroundImage, SDL_Color backgroundColor, SDL_Color foregroundColor);
public:
	void Resize(GUISize size);

	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
	void Text(const std::string& text);
	void BackgroundImage(const std::string& fileName);
};
