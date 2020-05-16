#include "GUITexture.h"
#include "GUI.h"
#include "../exception/GUIException.h"

GUITexture::GUITexture(): 
	mode_(blendMode::none)
{
	texture_ = nullptr;
	with_ = 0;
	height_ = 0;
}

GUITexture::GUITexture(SDL_Texture* texture, int with, int height):
	mode_(blendMode::none)
{
	texture_ = texture;
	with_ = with;
	height_ = height;
}

GUITexture::GUITexture(SDL_Texture* texture, GUISize size):
	mode_(blendMode::none)
{
	texture_= texture;
	with_ = size.width;
	height_ = size.height;
}

GUITexture::~GUITexture() {
    if (texture_ != nullptr) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}

GUITexture::GUITexture(GUITexture& that):
    mode_(that.mode_) {
    texture_ = that.texture_;
    with_ = that.with_;
    height_ = that.height_;

    that.Detach();
}

GUITexture& GUITexture::operator=(GUITexture& that)
{
	if (this != &that)
	{
		texture_ = that.texture_;
		with_ = that.with_;
		height_ = that.height_;
		mode_ = that.mode_;
		that.Detach();
	}
	return *this;
}

GUISize GUITexture::Size() const
{
	return GUISize(with_, height_);
}

GUITexture::operator SDL_Texture*() const
{
	return texture_;
}

bool GUITexture::IsValid() const
{
	return texture_ != nullptr && with_ > 0 && height_ > 0;
}

void GUITexture::SetBlendMode(blendMode mode)
{
	mode_ = mode;
	SDL_BlendMode internMode;
	switch (mode)
	{
		case blendMode::none:
			internMode = SDL_BLENDMODE_NONE;
			break;
		case blendMode::blend:
			internMode = SDL_BLENDMODE_BLEND;
			break;
		case blendMode::add:
			internMode = SDL_BLENDMODE_ADD;
			break;
		case blendMode::mod:
			internMode = SDL_BLENDMODE_MOD;
			break;
		default:
			throw GUIException("BlendMode not implemented");
	}

	if(SDL_SetTextureBlendMode(texture_, internMode) != 0)
	{
		throw GUIException("SDL_SetTextureBlendMode");
	}
}

//Todo remove this
void GUITexture::SetBlendMode(const SDL_BlendMode mode) {
	switch (mode)
	{
		case SDL_BLENDMODE_NONE:
			mode_ = blendMode::none;
			break;
		case SDL_BLENDMODE_BLEND:
			mode_ = blendMode::blend;
			break;
		case SDL_BLENDMODE_ADD:
			mode_ = blendMode::add;
			break;
		case SDL_BLENDMODE_MOD:
			mode_ = blendMode::mod;
			break;
		default:
			throw GUIException("BlendMode not implemented");
	}

	if (SDL_SetTextureBlendMode(texture_, mode) != 0)
	{
		throw GUIException("SDL_SetTextureBlendMode");
	}
}

void GUITexture::Resize(SDL_Texture* texture, const GUISize size)
{
	with_ = size.width;
	height_ = size.height;
	if(texture_ != nullptr)
	{
		SDL_DestroyTexture(texture_);
	}
	texture_ = texture;
	SetBlendMode(mode_);
}

void GUITexture::Assign(GUITexture& that)
{
	texture_ = that.texture_;
	with_ = that.with_;
	height_ = that.height_;
	mode_ = that.mode_;
	that.texture_ = nullptr;
}

void GUITexture::Detach()
{
	texture_ = nullptr;
}

void GUITexture::SetTextureAlphaMod(Uint8 alpha) {
	SDL_SetTextureAlphaMod(texture_, alpha );
}