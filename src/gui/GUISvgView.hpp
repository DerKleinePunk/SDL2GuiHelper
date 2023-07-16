#pragma once

#include "GUICairoTexture.hpp"
#include "GUIElement.h"

struct GUIPoint;
struct GUISize;
struct GUIEvent;

/**
 * @brief Draw an SVG as Control
 * 
 */
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
    GUISvgView(GUIPoint position, GUISize size, const std::string& name, const std::string& fileName);
    ~GUISvgView();

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;

    void SetFile(const std::string& fileName);
};
