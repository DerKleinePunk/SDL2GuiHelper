#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUIProgressbar"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "GUIProgressbar.h"

#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
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


#ifdef ENABLECAIRO
static void rounded_rectangle(cairo_t* cr, int x, int y, int w, int h, int r)
{
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + r, y + r, r, M_PI, 3.0 * M_PI / 2.0);
    cairo_arc(cr, x + w - r, y + r, r, 3.0 * M_PI / 2.0, 2.0 * M_PI);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2.0);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2.0, M_PI);
    cairo_close_path(cr);
}
#endif

void GUIProgressbar::DrawIntern()
{
#ifdef ENABLECAIRO
    auto image = _cairoTexture->GetCairo();

    /* Paint background transparent*/
    cairo_set_source_rgba(image, 1.0, 1.0, 1.0, 0.0); /* white full tranparent */
    cairo_paint(image);

    // Draw block
    cairo_set_source_rgba(image, 0.0, 0.0, 0.0, 1.0); /* green half transparent*/
    cairo_rectangle(image, 8, 8, Size().width - 16, Size().height - 16);
    cairo_fill(image);

    double linesize = 5.0;
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

    cairo_pattern_destroy(pattern);

    //cairo_rectangle(image, 5, 5, Size().width - 10, Size().height - 10);
    //cairo_set_line_join(image, CAIRO_LINE_JOIN_ROUND); 
    //cairo_stroke(image);

    // rounded_rectangle (image, 15, 15, Size().width-30, Size().height-30, 10);
    // cairo_fill (image);

    /*double x = 25.6;
    double y = 25.6;
    double w         = 204.8;
    double h        = 204.8;
    double r = 15.0;
    double aspect        = 1.0;//aspect ratio
    //double corner_radius = height;

    double radius = corner_radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (image);
    cairo_arc (image, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_arc (image, x + w - r, y + r, r, 3 *M_PI / 2, 2 * M_PI);
    cairo_arc (image, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc (image, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_arc (image, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (image, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (image, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (image, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);

    cairo_close_path (image);

    cairo_set_source_rgb (image, 0.5, 0.5, 1);
    cairo_fill_preserve (image);
    cairo_set_source_rgba (image, 0.5, 0, 0, 0.5);
    cairo_set_line_width (image, 10.0);
    cairo_stroke (image);*/

    // Draw Background
    // Aufpassen wert zwischen 0.0 und 1.0
    // cairo_set_source_rgba(image, 1.0, 0.0, 0.0, 1.0);
    // cairo_rectangle(image, 0, 0, Size().width, Size().height);
    // cairo_fill(image);

    
    _cairoTexture->PaintDone();
#endif
}

GUIProgressbar::GUIProgressbar(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor)
    : GUIElement(position, size, name), GUIOnClickDecorator(static_cast<GUIElement*>(this))
{
    _logger = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = transparent_color;
    _background = background;
    _textcolor = textcolor;
}

GUIProgressbar::~GUIProgressbar()
{
}

void GUIProgressbar::Init()
{
    // Things after Control is Created
    _value = 100;
    _valueIntern = 1.0;
#ifdef ENABLECAIRO
    _cairoTexture = new GUICarioTexture(renderer_, Size());
    DrawIntern();
#endif
}

void GUIProgressbar::Draw()
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

void GUIProgressbar::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUIProgressbar::UpdateAnimation()
{
}

void GUIProgressbar::Close()
{
}

void GUIProgressbar::ButtonDownUpdate(Uint8 button)
{
    UNUSED(button);
}

void GUIProgressbar::ButtonUpUpdate(Uint8 button)
{
    UNUSED(button);
}

void GUIProgressbar::SetValue(unsigned char value)
{
    if(_value == value) return;

    _value = value;
    _valueIntern = value / 100.0;
    DrawIntern();
    SetRedraw();
}