#pragma once

struct GUIEvent;
class GUITexture;

#include "GUIOnClickDecorator.h"

class GUIIconview : public GUIElement, public GUIOnClickDecorator {
    RTTI_DERIVED(GUIIconview);
    el::Logger* logger_;
	GUITexture* imageTexture_;
public:
	GUIIconview(GUIPoint position, GUISize size, const std::string& name, SDL_Color background);

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;

	void SetCurrentIcon(const std::string& fileName);

    void ButtonDownUpdate(Uint8 button) override;
	void ButtonUpUpdate(Uint8 button) override;
};
