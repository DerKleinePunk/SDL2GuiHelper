//
// Created by punky on 14.06.22.
//

#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUIProgressbar"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "GUIGauge.h"

#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
#include "GUICairoTexture.hpp"
#include "GUIElement.h"
#include "GUIImageManager.h"
#include "GUIRenderer.h"
#include "GUITexture.h"

#ifdef ENABLECAIRO
#if defined(__WIN32__) || defined(WIN32)
#include <cairo.h>
#else
#include <cairo/cairo.h>
#endif
#include <pango/pango-glyph.h>
#include <pango/pangocairo.h>
#endif

void GUIGauge::DrawIntern()
{
#ifdef ENABLECAIRO
    auto image = _cairoTexture->GetCairo();

    double radius = Size().height / 2 - 32 ;
    double angle1 = 160.0  * (M_PI/180.0);  /* angles are specified */
    double angle2 = 380.0 * (M_PI/180.0);  /* in radians           */

    /* Paint background transparent*/
    cairo_set_source_rgba(image, 1.0, 1.0, 1.0, 0.0); /* white full tranparent */
    cairo_paint(image);

    double linesize = 16.0;
    cairo_set_source_rgb(image, 1.0, 1.0, 1.0);
    cairo_set_line_width(image, linesize);
    cairo_arc(image, Size().width / 2, Size().height / 2, radius, angle1, angle2);
    cairo_stroke (image);

    angle1 = 161.0  * (M_PI/180.0);
    angle2 = 379.0 * (M_PI/180.0);
    linesize -= 3;
    cairo_set_source_rgb(image, 0.0, 0.0, 0.0);
    cairo_set_line_width(image, linesize);
    cairo_arc(image, Size().width / 2, Size().height / 2, radius, angle1, angle2);
    cairo_stroke (image);

    double angle3 = 240.0 * (M_PI/180.0);
    cairo_set_source_rgb(image, 0.0, 1.0, 0.0);
    cairo_set_line_width(image, linesize);
    cairo_arc(image, Size().width / 2, Size().height / 2, radius, angle1, angle3);
    cairo_stroke (image);

    /* Draw block
    cairo_set_source_rgba(image, 0.0, 0.0, 0.0, 1.0); /* black half transparent
    cairo_rectangle(image, 8, 8, Size().width - 16, Size().height - 16);
    cairo_fill(image);


    cairo_set_source_rgb(image, 0.0, 1.0, 0.0);
    // cairo_set_fill_rule (image, CAIRO_FILL_RULE_EVEN_ODD);
    //cairo_set_line_width(image, 5.0);
    rounded_rectangle(image, linesize, linesize, (Size().width-(linesize * 2.0)) * _valueIntern, Size().height-(linesize * 2.0), linesize);
    //rounded_rectangle(image, 15, 15, Size().width-30, Size().height-30, 10);
    auto pattern = cairo_pattern_create_linear(0, 0, 0, Size().height);
    cairo_pattern_add_color_stop_rgba (pattern, 0.00,  0, 1, 0, 0.1);
    cairo_pattern_add_color_stop_rgba (pattern, 0.15,  0, 1, 0, 0.8);
    cairo_pattern_add_color_stop_rgba (pattern, 0.5,  0, 1, 0, 1);
    cairo_pattern_add_color_stop_rgba (pattern, 0.85,  0, 1, 0, 0.8);
    cairo_pattern_add_color_stop_rgba (pattern, 1.0,  0, 1, 0, 0.1);
    cairo_set_source (image, pattern);
    //cairo_fill_preserve(image);
    cairo_fill(image);
    rounded_rectangle(image, linesize, linesize, Size().width-(linesize * 2.0), Size().height-(linesize * 2.0), linesize);
    cairo_set_line_width(image, linesize);
    cairo_set_source_rgb(image, 102.0 / 255.0, 102.0 / 255.0, 102.0 / 255.0);
    cairo_stroke(image);

    cairo_pattern_destroy(pattern);*/
    _cairoTexture->PaintDone();
#endif
}

GUIGauge::GUIGauge(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor)
    : GUIElement(position, size, name), GUIOnClickDecorator(static_cast<GUIElement*>(this))
{
    _logger = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = transparent_color;
    _background = background;
    _textcolor = textcolor;
}

GUIGauge::~GUIGauge()
{
#ifdef ENABLECAIRO
    if(_cairoTexture != nullptr) {
        delete _cairoTexture;
    }
#endif
}

void GUIGauge::Init()
{
    // Things after Control is Created
    _value = 100;
    _valueIntern = 1.0;
#ifdef ENABLECAIRO
    _cairoTexture = new GUICairoTexture(renderer_, Size());
    DrawIntern();
#endif
}

void GUIGauge::Draw()
{
#ifdef ENABLECAIRO
    auto texture = _cairoTexture->GetTexture();
    if(texture != nullptr) {
        auto drawTextPosition = GUIPoint(0, 0);
        renderer_->RenderCopy(texture, drawTextPosition);
    }
#endif

    needRedraw_ = false;
}

void GUIGauge::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUIGauge::UpdateAnimation()
{
}

void GUIGauge::Close()
{
}

void GUIGauge::ButtonDownUpdate(Uint8 button, const GUIPoint& point)
{
    UNUSED(button);
}

void GUIGauge::ButtonUpUpdate(Uint8 button, const GUIPoint& point)
{
    UNUSED(button);
}

void GUIGauge::ButtonMoveUpdate(Uint8 button, const GUIPoint& point)
{
    UNUSED(button);
}

void GUIGauge::SetValue(unsigned char value)
{
    if(_value == value) return;

    _value = value;
    _valueIntern = value / 100.0;
    DrawIntern();
    SetRedraw();
}

