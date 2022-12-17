#pragma once
#pragma once

#include "GUICairoTexture.hpp"
#include "GUIElement.h"

struct GUIPoint;
struct GUISize;
struct GUIEvent;

class GUIArtificialHorizon : public GUIElement
{
private:
    el::Logger* _logger;
    double _rolling;
#ifdef ENABLECAIRO
    GUICairoTexture* _cairoTexture;
#endif
    void DrawIntern();
    RTTI_DERIVED(GUIArtificialHorizon);
public:
    GUIArtificialHorizon(GUIPoint position, GUISize size, const std::string& name);
    ~GUIArtificialHorizon();

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;

    void SetRolling(double value);
};


