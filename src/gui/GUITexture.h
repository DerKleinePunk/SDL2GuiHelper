#pragma once
#include <SDL.h>

enum class blendMode : unsigned char;
struct GUISize;

class  GUITexture {
	int with_;
	int height_;
	blendMode mode_;
	SDL_Texture* texture_;
public:
	GUITexture();
	GUITexture(SDL_Texture* texture, int with, int height);
	GUITexture(SDL_Texture* texture, GUISize size);
	~GUITexture();

	GUITexture(GUITexture& that);//copy constructor
	GUITexture& operator=(GUITexture& that);//copy assignment operator

	GUISize Size() const;
	operator SDL_Texture*() const;
	bool IsValid() const;
	void SetBlendMode(blendMode mode);
	void SetBlendMode(SDL_BlendMode mode);
	void Resize(SDL_Texture* texture, GUISize size);
	void Assign(GUITexture& that);
	void Detach();
	void SetTextureAlphaMod(Uint8 alpha);
};






