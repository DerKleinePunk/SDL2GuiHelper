#pragma once
#include "GUIElement.h"

class GUIScreenCanvas : public GUIElement
{
	friend class GUIScreen;
	std::string _backgroundImage;
	GUITexture* imageTexture_;
	RTTI_DERIVED(GUIScreen);

protected:
	explicit GUIScreenCanvas(GUISize size, const std::string& backgroundImage);
public:
	void Resize(GUISize size);

	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
};
