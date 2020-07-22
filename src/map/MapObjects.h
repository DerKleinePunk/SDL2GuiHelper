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

struct GeoLabel {
  std::string type;
  osmscout::GeoCoord position;
};

class MapObjects
{
private:
    cairo_t* _mapImage;
    std::unordered_map<size_t,PangoFontDescription*> _fontMap;
    std::unordered_map<std::string, GeoLabel> _labelList;

    PangoFontDescription* GetFont(double size,std::string fontName);
public:
    MapObjects(cairo_t* mapImage);
    ~MapObjects();

    void DrawFilledLabel(double x, double y, std::string text);
    void DrawLabel(std::string type, double x, double y);
    void DrawAll(osmscout::MercatorProjection projection);
    void SetTargetPos(const double& lat,const double& lon);
};


