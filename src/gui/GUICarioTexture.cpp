#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUICarioTexture"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
#include "GUICarioTexture.hpp"

#ifdef ENABLECAIRO

void GUICarioTexture::Create()
{
    _texturePixels = new unsigned char[_size.width * _size.height * 4];
    _image_data_source = cairo_image_surface_create_for_data(_texturePixels, CAIRO_FORMAT_ARGB32, _size.width, _size.height,
                                        cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, _size.width));
    _cairoImage = cairo_create(_image_data_source);
}

GUICarioTexture::GUICarioTexture(GUIRenderer* renderer, GUISize size)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _image_data_source = nullptr;
    _cairoImage = nullptr;
    _texturePixels = nullptr;
    _texture = nullptr;
    _size = size;
    _renderer = renderer;
}

GUICarioTexture::~GUICarioTexture()
{
    //Todo aufräumen speicher
    if(_texturePixels != nullptr){
        delete _texturePixels;
    }
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
    cairo_surface_flush(_image_data_source);

    if(_texture == nullptr){
        _texture = _renderer->CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _size);
    }

    void *pixels;
    int pitch;
    //SDL_LockMutex(mapMemLock_);
    if(SDL_LockTexture(_texture->operator SDL_Texture*(), NULL, &pixels, &pitch) == 0) {
        memcpy(pixels, _texturePixels, pitch * _size.height);
        SDL_UnlockTexture(_texture->operator SDL_Texture*());
    } else {
        LOG(ERROR) << SDL_GetError();
    }
}

GUITexture* GUICarioTexture::GetTexture()
{
    return _texture;
}

#endif // ENABLECAIRO