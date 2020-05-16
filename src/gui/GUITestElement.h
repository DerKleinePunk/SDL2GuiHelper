#pragma once

struct GUIPoint;
struct GUISize;
struct GUIEvent;

class GUITestElement : public GUIElement
{
	bool transparency_;
	GUITexture* imageTextureBackground_;

	RTTI_DERIVED(GUITestElement);

public:
	GUITestElement(GUIPoint position, GUISize size, const std::string& name);
	GUITestElement(GUIPoint position, GUISize size, SDL_Color background, const std::string& name);
		
	void Transparent();
	void ImageBackground(std::string fileName);

	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
};
