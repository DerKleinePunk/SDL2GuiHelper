#pragma once

class GUIRoundPictureButton : public GUIElement, public GUIOnClickDecorator
{
	RTTI_DERIVED(GUIRoundPictureButton);
	el::Logger* logger_;
	int centerX_;
	int centerY_;
	bool _buttonDown;
	SDL_Color backgroundColorCircle_;
	GUITexture* imageTexture_;
	GUITexture* imageTextureDisable_;
	GUITexture* imageTextureBackground_;
	GUITexture* imageSelected_;
	GUITexture* imageSelectedBackground_;
	GUISize pictureSize_;
public:
	GUIRoundPictureButton(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);

	void Image(std::string fileName);
	void ImageDisable(std::string fileName);
	void ImageBackground(std::string fileName);
	void ImageSelected(std::string fileName);
	void ImageSelectedBackground(std::string fileName);

	void Border(bool on);
	void PictureSize(const GUISize& size);

	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
	void ButtonDownUpdate(Uint8 button, const GUIPoint& point) override;
	void ButtonUpUpdate(Uint8 button, const GUIPoint& point) override;
    void ButtonMoveUpdate(Uint8 button, const GUIPoint& point) override;
};


