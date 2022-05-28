#include "GUI.h"
#include "GUICarioTexture.hpp"

#ifdef ENABLECAIRO

void GUICarioTexture::Create()
{
    _texturePixels = new unsigned char[_size.width * _size.height * 4];
    cairo_image_surface_create_for_data(_texturePixels, CAIRO_FORMAT_ARGB32, _size.width, _size.height,
                                        cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, _size.width));
    _cairoImage = cairo_create(_image_data_source);
}

GUICarioTexture::GUICarioTexture(GUISize size)
{
    _image_data_source = nullptr;
    _cairoImage = nullptr;
    _texturePixels = nullptr;
    _texture = nullptr;
    _size = size;
}

GUICarioTexture::~GUICarioTexture()
{
    //Todo aufräumen speicher
    if(_texturePixels != nullptr)[
        delete [] _texturePixels;
    ]
}

cairo_t* GUICarioTexture::GetCairo()
{
    if(_image_data_source == nullptr) {
        Create();
    }

    return _cairoImage;
}

void GUICarioTexture::PaintDone()
{
}

GUITexture* GUICarioTexture::GetTexture()
{
}

#endif // ENABLECAIRO