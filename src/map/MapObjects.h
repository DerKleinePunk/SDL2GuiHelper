#pragma once

#include <string>
#include <unordered_map>

#if defined(__WIN32__) || defined(WIN32)
  #include <cairo.h>
#else
  #include <cairo/cairo.h>
#endif

#include <pango/pangocairo.h>
#include <pango/pango-glyph.h>

#include <osmscout/util/Projection.h>

class MapObjects
{
private:
    cairo_t* _mapImage;
    std::unordered_map<size_t,PangoFontDescription*> _fontMap;

    PangoFontDescription* GetFont(double size,std::string fontName);
public:
    MapObjects(cairo_t* mapImage);
    ~MapObjects();

    void DrawFilledLabel(double x, double y, std::string text);
    void DrawAll(osmscout::MercatorProjection projection);
};


