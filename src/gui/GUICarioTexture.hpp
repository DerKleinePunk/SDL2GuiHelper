#pragma once

#ifdef ENABLECAIRO

#if defined(__WIN32__) || defined(WIN32)
  #include <cairo.h>
#else
  #include <cairo/cairo.h>
#endif

#include <pango/pangocairo.h>
#include <pango/pango-glyph.h>
#include "GUITexture.h"
#include "GUIRenderer.h"

class GUICarioTexture
{
private:
    cairo_surface_t* _image_data_source;
    cairo_t* _cairoImage;
    unsigned char* _texturePixels;
    GUISize _size;
    GUITexture* _texture;
    GUIRenderer* _renderer;
    void Create();
public:
    GUICarioTexture(GUIRenderer* renderer, GUISize size);
    ~GUICarioTexture();

    cairo_t* GetCairo();
    void PaintDone();

    GUITexture* GetTexture();
};

#endif //ENABLECAIRO