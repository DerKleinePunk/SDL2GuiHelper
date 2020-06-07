#pragma once
#include <SDL_ttf.h>

struct GUIPoint;
struct GUISize;
struct GUIEvent;
class GUITexture;

typedef std::function<void(IGUIElement* sender)> FocusDelegate;

class GUITextEdit : public GUIElement
{
    Uint32 lastCursorTick_;
    bool cursorOn_;
    GUITexture* textureText_;
    std::string text_ ;
    TTF_Font* font_;
    FocusDelegate _OnFocus;
    
    void RenderText();
    void ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point);

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
    void SetText(const std::string& text);
    void RegisterOnFocus(FocusDelegate OnFocus);
};

