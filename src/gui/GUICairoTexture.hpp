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

class GUICairoTexture
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
  GUICairoTexture(GUIRenderer* renderer, GUISize size);
    ~GUICairoTexture();

    cairo_t* GetCairo();
    void PaintDone();

    GUITexture* GetTexture();

    void RoundedRectangle(int x, int y, int w, int h, int r);
    int LoadSvg(const std::string& fileName, const GUIRect& viewPort);
    /**
     * @brief LoadSvg from Memory
     * 
     * @param buffer this buffer
     * @param bufferSize Buffersize in Bytes
     * @param viewPort 
     * @return int 0 no error
     */
    int LoadSvg(const unsigned char *buffer, size_t bufferSize, const GUIRect& viewPort);
};

#endif //ENABLECAIRO