//
// Created by punky on 14.06.22.
//
#pragma once
#ifndef SDL2GUIHELPER_GUIGAUGE_H
#define SDL2GUIHELPER_GUIGAUGE_H

#include <SDL_ttf.h>

#include "GUICairoTexture.hpp"
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"

struct GUIPoint;
struct GUISize;
struct GUIEvent;
class GUITexture;

class GUIGauge : public GUIElement, public GUIOnClickDecorator
{
    SDL_Color _background;
    SDL_Color _textcolor;
    el::Logger* _logger;
    unsigned char _value;
    double _valueIntern;
#ifdef ENABLECAIRO
    GUICairoTexture* _cairoTexture;
#endif
    void DrawIntern();
    RTTI_DERIVED(GUIGauge);
  public:
    GUIGauge(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor);
    ~GUIGauge();

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;

    void ButtonDownUpdate(Uint8 button, const GUIPoint& point) override;
    void ButtonUpUpdate(Uint8 button, const GUIPoint& point) override;
    void ButtonMoveUpdate(Uint8 button, const GUIPoint& point) override;

    void SetValue(unsigned char value);
};


#endif // SDL2GUIHELPER_GUIGAUGE_H
