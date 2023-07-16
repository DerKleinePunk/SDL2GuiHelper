#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUICairoTexture"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "GUICairoTexture.hpp"

#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
#ifdef ENABLECAIRO

#include <rsvg.h>

void GUICairoTexture::Create()
{
    _texturePixels = new unsigned char[_size.width * _size.height * 4];
    _image_data_source = cairo_image_surface_create_for_data(_texturePixels, CAIRO_FORMAT_ARGB32, _size.width, _size.height,
                                        cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, _size.width));
    _cairoImage = cairo_create(_image_data_source);
}

GUICairoTexture::GUICairoTexture(GUIRenderer* renderer, GUISize size)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _image_data_source = nullptr;
    _cairoImage = nullptr;
    _texturePixels = nullptr;
    _texture = nullptr;
    _size = size;
    _renderer = renderer;
}

GUICairoTexture::~GUICairoTexture()
{
    if(_texture != nullptr) {
        delete _texture;
    }

    if(_cairoImage != nullptr) {
        cairo_destroy(_cairoImage);
    }

    if(_image_data_source != nullptr) {
        cairo_surface_destroy(_image_data_source);
    }

    if(_texturePixels != nullptr){
        delete [] _texturePixels;
    }
}

cairo_t* GUICairoTexture::GetCairo()
{
    if(_image_data_source == nullptr) {
        Create();
    }

    return _cairoImage;
}

void GUICairoTexture::PaintDone()
{
    if(_renderer == nullptr) {
        LOG(WARNING) << "_renderer is null wrong init State ?";
        return;
    }

    if(_image_data_source == nullptr) {
        LOG(WARNING) << "no data to paint";
        return;
    }

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

GUITexture* GUICairoTexture::GetTexture()
{
    return _texture;
}

void GUICairoTexture::RoundedRectangle(int x, int y, int w, int h, int r)
{
    if(_image_data_source == nullptr) {
        Create();
    }

    cairo_new_sub_path(_cairoImage);
    cairo_arc(_cairoImage, x + r, y + r, r, M_PI, 3.0 * M_PI / 2.0);
    cairo_arc(_cairoImage, x + w - r, y + r, r, 3.0 * M_PI / 2.0, 2.0 * M_PI);
    cairo_arc(_cairoImage, x + w - r, y + h - r, r, 0, M_PI / 2.0);
    cairo_arc(_cairoImage, x + r, y + h - r, r, M_PI / 2.0, M_PI);
    cairo_close_path(_cairoImage);
}

int GUICairoTexture::LoadSvg(const std::string& fileName, const GUIRect& viewPort )
{
    if(_image_data_source == nullptr) {
        Create();
    }

    GError *error = nullptr;
    auto handle = rsvg_handle_new_from_file(fileName.c_str(), &error);
    if(error != nullptr ) {
        LOG(ERROR) << error->message;
        return -1;
    }

    RsvgRectangle viewport = { 0.0, 0.0, 0.0, 0.0 };
    viewport.x = viewPort.x;
    viewport.y = viewPort.y;
    viewport.width = viewPort.w;
    viewport.height = viewPort.h; 

    if(rsvg_handle_render_document (handle, _cairoImage, &viewport, &error) == FALSE) {
        LOG(ERROR) << "Render SVG Failed";
    }

    if(error != nullptr ) {
        LOG(ERROR) << error->message;
        g_object_unref(handle);
        return -1;
    }

    g_object_unref(handle);

    return 0;
}

int GUICairoTexture::LoadSvg(const unsigned char *buffer, size_t bufferSize, const GUIRect& viewPort)
{
    if(_image_data_source == nullptr) {
        Create();
    }

    GError *error = nullptr;
    auto handle = rsvg_handle_new_from_data(buffer, bufferSize, &error);
    if(error != nullptr ) {
        LOG(ERROR) << error->message;
        return -1;
    }
    
    /*
    const char *css = "svg#svg821 { max-width: 100%; height: auto; max-height: 90vh; transform: scale(2.2); }";
    
    if(rsvg_handle_set_stylesheet (handle, (const guint8 *) css, strlen (css), &error) == FALSE)  {
        LOG(ERROR) << "set stylesheet";
        if(error != nullptr ) {
            LOG(ERROR) << error->message;
        }
    }*/

    RsvgRectangle viewport = { 0.0, 0.0, 0.0, 0.0 };
    viewport.x = viewPort.x;
    viewport.y = viewPort.y;
    viewport.width = viewPort.w;
    viewport.height = viewPort.h; 

    if(rsvg_handle_render_document(handle, _cairoImage, &viewport, &error) == FALSE) {
        LOG(ERROR) << "render document";
        if(error != nullptr ) {
            LOG(ERROR) << error->message;
        }
    }

    if(error != nullptr ) {
        g_object_unref(handle);
        
        //Todo Log Error
        return -1;
    }


    g_object_unref(handle);

    return 0;
}

#endif // ENABLECAIRO