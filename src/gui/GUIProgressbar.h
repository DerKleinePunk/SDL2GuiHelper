#pragma once
#include <SDL_ttf.h>
#include "GUIOnClickDecorator.h"
#include "GUICarioTexture.hpp"

struct GUIPoint;
struct GUISize;
struct GUIEvent;
class GUITexture;

class GUIProgressbar : public GUIElement, public GUIOnClickDecorator
{
    SDL_Color _background;
    SDL_Color _textcolor;
    el::Logger* logger_;
#ifdef ENABLECAIRO    
    GUICarioTexture* _cairoTexture;
#endif
    RTTI_DERIVED(GUIProgressbar);
public:
    GUIProgressbar(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);
    ~GUIProgressbar();

    void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;

    void ButtonDownUpdate(Uint8 button) override;
	void ButtonUpUpdate(Uint8 button) override;
};