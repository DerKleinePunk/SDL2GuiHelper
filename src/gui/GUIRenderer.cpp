#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIRenderer"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "../exception/TTFException.h"
#include "../../common/exception/NotImplementedException.h"
#include "GUIRenderer.h"
#include "../exception/GUIException.h"
#include "../../common/exception/NullPointerException.h"
#include "../../common/utils/Tokenizer.h"
#include "GUITexture.h"
#include <SDL_image.h>

void GUIRenderer::CheckThreadId() const {
	if(_guiThreadId != std::this_thread::get_id()) {
		throw GUIException("Wrong Thread Id");
	}
}

void GUIRenderer::Create(SDL_Window* window)
{
	renderer_ = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer_ == nullptr)
	{
		throw GUIException("SDL_CreateRenderer");
	}
    
	windowPixleFormat_ = SDL_GetWindowPixelFormat(window);

    SDL_RendererInfo info;
    if(SDL_GetRendererInfo(renderer_,&info) != 0) {
        throw GUIException("SDL_GetRendererInfo");
    }
    max_texture_height_ = info.max_texture_height;
    max_texture_width_ = info.max_texture_width;
    LOG(INFO) << "Max Renderer Size is " << max_texture_height_ << "x" << max_texture_width_;
}

void GUIRenderer::SetViewport(GUIRect rect) const
{
	if(SDL_RenderSetViewport(renderer_, &rect) != 0)
	{
		throw GUIException("RenderSetViewport");
	}
}

void GUIRenderer::SetClipRect(GUIRect rect) const {
	if (SDL_RenderSetClipRect(renderer_, &rect) != 0) {
		throw GUIException("RenderSetClipRect");
	}
}
void GUIRenderer::ClearClipRect() const {
    if (SDL_RenderSetClipRect(renderer_, nullptr) != 0) {
		throw GUIException("RenderSetClipRect null");
	}
}

void GUIRenderer::DrawFillRect(GUIRect rect) const {
	if (rect.w && rect.h) {
		if (SDL_RenderFillRect(renderer_, &rect) != 0) {
			throw GUIException("SDL_RenderFillRect");
		}
	}
}

void GUIRenderer::DrawRoundFillRect(GUIRect rect, int radius) const {
	Uint8 r,g,b,a;
	if(SDL_GetRenderDrawColor(renderer_, &r, &g, &b, &a) != 0) {
		throw GUIException("SDL_GetRenderDrawColor");
	}

	if(radius <= 2) {
		DrawFillRect(rect);
		return;
	}
	SDL_Color color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	DrawRoundFillRect(rect, radius, color);
}

void GUIRenderer::DrawFillRect(GUIRect rect, const SDL_Color color) const
{
	if (rect.w && rect.h) {
		DrawColor(color);
		if (SDL_RenderFillRect(renderer_, &rect) != 0) {
			throw GUIException("SDL_RenderFillRect");
		}
	}
}

void GUIRenderer::DrawRoundFillRect(GUIRect rect, int radius, SDL_Color color) const {
	if(roundedBoxRGBA(renderer_, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h -1, radius, color.r, color.g, color.b, color.a) != 0){
		throw GUIException("roundedBoxRGBA");
	}
}

void GUIRenderer::DrawRect(GUIRect rect) const {
	if (rect.w && rect.h) {
		if (SDL_RenderDrawRect(renderer_, &rect) != 0) {
			throw GUIException("SDL_RenderDrawRect");
		}
	}
}

void GUIRenderer::DrawRoundRect(GUIRect rect, int radius) const {
	Uint8 r,g,b,a;
	if(SDL_GetRenderDrawColor(renderer_, &r, &g, &b, &a) != 0) {
		throw GUIException("SDL_GetRenderDrawColor");
	}

	if(radius <= 2) {
		DrawRect(rect);
		return;
	}
	SDL_Color color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	DrawRoundFillRect(rect, radius, color);
}

void GUIRenderer::DrawRect(GUIRect rect, const SDL_Color color) const {
	if (rect.w && rect.h) {
		DrawColor(color);
		if (SDL_RenderDrawRect(renderer_, &rect) != 0) {
			throw GUIException("SDL_RenderDrawRect");
		}
	}
}

void GUIRenderer::DrawRoundRect(GUIRect rect, int radius, SDL_Color color) const {
	if (rect.w && rect.h) {
		DrawColor(color);
		if (roundedRectangleRGBA(renderer_, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, radius, color.r, color.g, color.b, color.a) != 0) {
			throw GUIException("roundedRectangleRGBA");
		}
	}
}

void GUIRenderer::Clear(const SDL_Color color) const
{
	DrawColor(color);
	if(SDL_RenderClear(renderer_) != 0)
	{
		throw GUIException("SDL_RenderClear");
	}
}

void GUIRenderer::Present() const
{
	CheckThreadId();
	SDL_RenderPresent(renderer_);
}

void GUIRenderer::DrawColor(SDL_Color color) const
{
	CheckThreadId();
	DrawColor(color.r, color.g, color.b, color.a);
}

void GUIRenderer::DrawColor(Uint32 color) const
{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	color = byteswap32(color);
#endif
	auto colorValue = reinterpret_cast<Uint8 *>(&color);

	DrawColor(colorValue[0], colorValue[1], colorValue[2], colorValue[3]);
}

void GUIRenderer::DrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) const
{
	CheckThreadId();
	if (SDL_SetRenderDrawBlendMode(renderer_, (alpha == 0xFF) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND) != 0)
	{
		throw GUIException("SDL_SetRenderDrawBlendMode");
	}

	if (SDL_SetRenderDrawColor(renderer_, r, g, b, alpha) != 0)
	{
		throw GUIException("SDL_SetRenderDrawColor");
	}
}

GUITexture* GUIRenderer::CreateTexture(GUISize size) const {
    return CreateTexture(windowPixleFormat_, SDL_TEXTUREACCESS_TARGET, size);
    //return CreateTexture(SDL_PIXELFORMAT_ARGB8888 / SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET | SDL_TEXTUREACCESS_STATIC SDL_TEXTUREACCESS_STREAMING, size);
}

void GUIRenderer::ResizeTexture(GUITexture* texture, GUISize size) const {
	Uint32 format;
	int access;
	SDL_QueryTexture(texture->operator SDL_Texture*(), &format, &access, nullptr, nullptr);
	auto result = SDL_CreateTexture(renderer_, format, access, size.width, size.height);
	if (result == nullptr) {
		throw GUIException("SDL_CreateTexture");
	}
	texture->Resize(result, size);
}

GUITexture* GUIRenderer::CreateTexture(Uint32 format, int access, GUISize size) const
{
	const auto result = SDL_CreateTexture(renderer_, format, access, size.width, size.height);
	if (result == nullptr)
	{
		throw GUIException("SDL_CreateTexture");
	}
	return new GUITexture(result, size);
}

void GUIRenderer::RenderTarget(GUITexture* texture) const
{
	CheckThreadId();

	SDL_Texture* textureLocal =  nullptr;
	if(texture != nullptr )
	{
		textureLocal = texture->operator SDL_Texture*();
	}

	if (SDL_SetRenderTarget(renderer_, textureLocal) != 0) {
		throw GUIException("SDL_SetRenderTarget");
	}
}

SDL_Texture* GUIRenderer::GetRenderTarget() const {
	return SDL_GetRenderTarget(renderer_);
}

void GUIRenderer::RenderCopy(GUITexture* texture, const GUIPoint topleft) const {
	if (texture == nullptr || !texture->IsValid()) {
		throw GUIException("nullptr texture");
	}

	const GUIRect rect(topleft, texture->Size());
	RenderCopy(texture, rect);
}

void GUIRenderer::RenderCopy(GUITexture* texture, GUIRect dstrect) const {
	CheckThreadId();
	if(texture == nullptr || !texture->IsValid())
	{
		throw GUIException("nullptr texture");
	}

	if (SDL_RenderCopy(renderer_, texture->operator SDL_Texture*(), nullptr, &dstrect) != 0) {
		throw GUIException("SDL_RenderCopy");
	}
}

void GUIRenderer::RenderCopy(GUITexture* texture, const GUIPoint topleft, const double angle, const GUIPoint pointToRotate) const {
	if (texture == nullptr || !texture->IsValid()) {
		throw GUIException("nullptr texture");
	}

	const GUIRect rect(topleft, texture->Size());
	RenderCopy(texture, rect, angle, pointToRotate);
}

void GUIRenderer::RenderCopy(GUITexture* texture, GUIRect dstrect, const double angle, GUIPoint pointToRotate) const {
	CheckThreadId();
	if (texture == nullptr || !texture->IsValid()) {
		throw GUIException("nullptr texture");
	}

	if (SDL_RenderCopyEx(renderer_, texture->operator SDL_Texture*(), nullptr, &dstrect, angle, &pointToRotate, SDL_FLIP_NONE) != 0) {
		throw GUIException("SDL_RenderCopyEx");
	}
}

SDL_RendererInfo GUIRenderer::GetInfo() const {
	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer_, &info) != 0) {
		throw GUIException("SDL_RenderCopyEx");
	}
	return info;
}

GUITexture* GUIRenderer::LoadTextureImageData(SDL_RWops* imageRaw) {
    const auto image = IMG_LoadTexture_RW(renderer_, imageRaw, 1);
    if(image == nullptr) {
        throw GUIException("IMG_LoadTexture_RW");
    }
    int w, h;
    SDL_QueryTexture(image, NULL, NULL, &w, &h);
    return new GUITexture(image, GUISize(w,h));
}

void GUIRenderer::DrawLine(int x1, int y1, int x2, int y2, SDL_Color color) const {
	CheckThreadId();
	DrawColor(color);
	if (SDL_RenderDrawLine(renderer_, x1, y1, x2, y2) != 0)	{
		throw GUIException("SDL_RenderDrawLine");
	}
}

void GUIRenderer::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) const {
	CheckThreadId();
	DrawColor(r, g, b, alpha);
	if (SDL_RenderDrawLine(renderer_, x1, y1, x2, y2) != 0)	{
		throw GUIException("SDL_RenderDrawLine");
	}
}

void GUIRenderer::DrawPixel(int x, int y, Uint32 color) const {
	CheckThreadId();
	DrawColor(color);
	if (SDL_RenderDrawPoint(renderer_, x, y) != 0) {
		throw GUIException("SDL_RenderDrawPoint");
	}
}

void GUIRenderer::DrawPixel(int x, int y, SDL_Color color) const {
	CheckThreadId();
	DrawColor(color);
	if (SDL_RenderDrawPoint(renderer_, x, y) != 0) {
		throw GUIException("SDL_RenderDrawPoint");
	}
}

void GUIRenderer::DrawPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) const {
	CheckThreadId();
	DrawColor(r, g, b, alpha);
	if (SDL_RenderDrawPoint(renderer_, x, y) != 0) {
		throw GUIException("SDL_RenderDrawPoint");
	}
}

void GUIRenderer::DrawMidpointEllipse(int centerX, int centerY, int width, int height, SDL_Color color, bool filled, int lineWidth) const {
	CheckThreadId();
	if(filled) {
		if(filledEllipseRGBA(renderer_, centerX, centerY, width, height, color.r, color.g, color.b, color.a) < 0) {
			throw GUIException("filledEllipseRGBA");
		}
	} else {
		if(ellipseRGBA(renderer_, centerX, centerY, width, height, color.r, color.g, color.b, color.a) < 0) {
			throw GUIException("ellipseRGBA");
		}
		if(lineWidth >= 2) {
			if(ellipseRGBA(renderer_, centerX, centerY, width-1, height-1, color.r, color.g, color.b, color.a) < 0) {
			throw GUIException("ellipseRGBA");
			}
		}
	}
}

GUITexture* GUIRenderer::LoadImageTexture(std::string fielName) const {
	CheckThreadId();
	auto result = IMG_LoadTexture(renderer_, fielName.c_str());
	if (result == nullptr)
	{
		throw GUIException("IMG_LoadTexture");
	}
	
	int w, h; // texture width & height
	SDL_QueryTexture(result, nullptr, nullptr, &w, &h);
	
	auto internResult = new GUITexture(result, w, h);
	internResult->SetBlendMode(blendMode::blend);

	//auto surface = IMG_Load(fielName.c_str());

	return internResult;
}

GUITexture* GUIRenderer::CreateTextureFromSurface(SDL_Surface* surface) const {
	CheckThreadId();
	auto result = SDL_CreateTextureFromSurface(renderer_, surface);
	if(result == nullptr) {
		throw GUIException("SDL_CreateTextureFromSurface");
	}
	return new GUITexture(result, surface->w, surface->h);
}

GUITexture* GUIRenderer::RenderTextBlended(TTF_Font* font, std::string text, SDL_Color foregroundColor) const {
	if(text.size() == 0) return nullptr;
	
	if (font == nullptr) {
		throw NullPointerException("font can not be null");
	}
	
	auto surfaceText = TTF_RenderUTF8_Blended(font, text.c_str(), foregroundColor);
	
	if(surfaceText == nullptr) {
		throw TTFException("RenderUTF8_Blended");
	}

    LOG(DEBUG) << "RenderTextBlended " << text << " " << surfaceText->h << " " << surfaceText->w;
    if(surfaceText->w > max_texture_width_){
        //Todo Fix this
        LOG(ERROR) << "result surface is to width to render";
    }
    if(surfaceText->h > max_texture_height_){
        LOG(ERROR) << "result surface is to height to render";
    }
	
	auto textureText = CreateTextureFromSurface(surfaceText);
	textureText->SetBlendMode(blendMode::blend);
	SDL_FreeSurface(surfaceText);
	return textureText;
}

//Some day we do this ?
//https://gamedev.stackexchange.com/questions/46238/rendering-multiline-text-with-sdl-ttf
//TTF_LineSkip TTF_GlyphMetrics and draw all own ? To max Text Center and so an without gerade memory...
GUITexture* GUIRenderer::RenderTextBlendedWrapped(TTF_Font* font, std::string text, SDL_Color foregroundColor, Uint32 wrapLength) const {
	if (text.size() == 0) return nullptr;

	if (font == nullptr) {
		throw NullPointerException("font can not be null");
	}

	//Make Textture Only so width the max Text width
	int w = 0,h = 0;
	Uint32 maxTextWidth = 0;
	Tokenizer tokenizer(text, "\n");
	while(tokenizer.NextToken()) {
		
		if(TTF_SizeUTF8(font,tokenizer.GetToken().c_str(),&w,&h) != 0) {
			throw TTFException("SizeUTF8");
		}

		if((Uint32)w > maxTextWidth){
			maxTextWidth = w;
		}
	}

	if(maxTextWidth > wrapLength){
		maxTextWidth = wrapLength;
	}

	auto surfaceText = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), foregroundColor, maxTextWidth);
	if (surfaceText == nullptr) {
		throw TTFException("RenderUTF8_Blended_Wrapped");
	}
	auto textureText = CreateTextureFromSurface(surfaceText);
	textureText->SetBlendMode(blendMode::blend);
	SDL_FreeSurface(surfaceText);
	return textureText;
}

GUIRenderer::GUIRenderer():
	renderer_(nullptr), 
	max_texture_height_(0), 
	max_texture_width_(0),
	windowPixleFormat_(0) {
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	_guiThreadId = std::this_thread::get_id();
 
}

GUIRenderer::~GUIRenderer()
{
	if (renderer_ != nullptr)
	{
		SDL_DestroyRenderer(renderer_);
		renderer_ = nullptr;
	}
}
