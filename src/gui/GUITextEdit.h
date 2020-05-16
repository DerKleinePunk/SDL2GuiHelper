#pragma once
#include <SDL_ttf.h>

struct GUIPoint;
struct GUISize;
struct GUIEvent;
class GUITexture;

class GUITextEdit : public GUIElement
{
    Uint32 lastCursorTick_;
    bool cursorOn_;
    GUITexture* textureText_;
    std::string text_ ;
    TTF_Font* font_;
    
    void RenderText();
    
    RTTI_DERIVED(GUITextEdit);
public:
    GUITextEdit(GUIPoint position, GUISize size, SDL_Color background, const std::string& name);
    ~GUITextEdit();

    void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
    
    void Select();
    void Unselect();
    std::string GetText();
};

