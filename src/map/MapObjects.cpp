#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MapObjects"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MapObjects.h"
#include "../../common/easylogging/easylogging++.h"

PangoFontDescription* MapObjects::GetFont(double size, std::string fontName)
{
    // fontSize = fontSize * projection.ConvertWidthToPixel(parameter.GetFontSize());

    const auto f = _fontMap.find(size);

    if(f != _fontMap.end()) {
        return f->second;
    }

    PangoFontDescription* font = pango_font_description_new();

    pango_font_description_set_family(font, fontName.c_str());
    pango_font_description_set_absolute_size(font, size * PANGO_SCALE);

    return _fontMap.insert(std::make_pair(size, font)).first->second;
}

MapObjects::MapObjects(cairo_t* mapImage)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _mapImage = mapImage;
}

MapObjects::~MapObjects()
{
    for(const auto& entry : _fontMap) {
        if(entry.second != nullptr) {
            pango_font_description_free(entry.second);
        }
    }
}

void MapObjects::DrawFilledLabel(double x, double y, std::string text)
{
    auto font = GetFont(13.0, "Inconsolata");

    // Aufpassen wert zwischen 0.0 und 1.0
    int textWidth, textHeight;

    auto layout = pango_cairo_create_layout(_mapImage);
    pango_layout_set_text(layout, text.c_str(), -1);
    pango_layout_set_font_description(layout, font);
    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
    pango_layout_get_size(layout, &textWidth, &textHeight);

    double textWidthD = textWidth / PANGO_SCALE;
    double textHeightD = textHeight / PANGO_SCALE;
    textWidthD += 2.0;
    textHeightD += 2.0;

    // Draw Background
    // Aufpassen wert zwischen 0.0 und 1.0
    cairo_set_source_rgba(_mapImage, 1.0, 0.0, 0.0, 1.0);
    cairo_rectangle(_mapImage, x, y, textWidthD, textHeightD);
    cairo_fill(_mapImage);

    // DrawText
    cairo_set_source_rgba(_mapImage, 0.0, 0.0, 0.0, 1.0);
    cairo_move_to(_mapImage, x + 1, y - 1);
    pango_cairo_show_layout(_mapImage, layout);
    // cairo_stroke(_mapImage);

    g_object_unref(layout);

    // Draw Rahmen
    cairo_set_source_rgba(_mapImage, 0.0, 1.0, 0.0, 1.0);
    cairo_set_line_width(_mapImage, 1);
    cairo_rectangle(_mapImage, x, y, textWidthD, textHeightD);
    cairo_stroke(_mapImage);

    /*
    cairo_set_source_rgb (_mapImage, 0.0, 0.0, 0.0);
    cairo_select_font_face (_mapImage, "Inconsolata" , CAIRO_FONT_SLANT_NORMAL,
    CAIRO_FONT_WEIGHT_BOLD); cairo_set_font_size (_mapImage, 20.0); cairo_move_to (_mapImage, x-50,
    y-50); cairo_show_text (_mapImage, text.c_str());*/
}

void MapObjects::DrawLabel(std::string type, double x, double y)
{
}

void MapObjects::DrawAll(osmscout::MercatorProjection projection)
{
    //_mapObjects->DrawFilledLabel(x-50, y-50, "Michael");
    auto entry = _labelList.begin();
    while(entry != _labelList.end()) {
        if(projection.IsValidFor(entry->second.position)) {
            double x, y;

            projection.GeoToPixel(entry->second.position, x, y);
            DrawLabel(entry->second.type, x, y);
        }
        entry++;
    }
}

void MapObjects::SetTargetPos(const double& lat, const double& lon)
{
    auto entry = _labelList.find("TARGET");
    if(entry == _labelList.end()) {
        GeoLabel label;
        label.type = "TARGET";
        label.position.Set(lat, lon);
        _labelList.insert(std::make_pair(std::string("TARGET"), label));
    }
}