#pragma once

#include "GUICairoTexture.hpp"
#include "GUIElement.h"

struct GUIPoint;
struct GUISize;
struct GUIEvent;

class GUISvgView : public GUIElement
{
private:
    el::Logger* _logger;
    std::string _fileName;
    
#ifdef ENABLECAIRO
    GUICairoTexture* _cairoTexture;
#endif
    void DrawIntern();
    RTTI_DERIVED(GUISvgView);
public:
    GUISvgView(GUIPoint position, GUISize size, const std::string& name);
    ~GUISvgView();

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;

    
};
