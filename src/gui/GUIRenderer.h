#pragma once

class GUITexture;
#include "GUI.h"
#include "SDL_ttf.h"
#include <thread>

class GUIRenderer
{
	SDL_Renderer* renderer_;
    int max_texture_height_;
    int max_texture_width_;
	Uint32 windowPixleFormat_;
	std::thread::id _guiThreadId;
	void CheckThreadId() const;
public:
	GUIRenderer();
	virtual ~GUIRenderer();
	
	void Create(SDL_Window* window, bool vsync);
	void SetViewport(GUIRect rect) const;
	void SetClipRect(GUIRect rect) const;
    void ClearClipRect() const;
	void Clear(SDL_Color color = black_color) const;
	void Present() const;
	GUITexture* CreateTexture(GUISize size) const;
	void ResizeTexture(GUITexture* texture, GUISize size) const;
	GUITexture* CreateTexture(Uint32 format, int access, GUISize size) const;
	void RenderTarget(GUITexture* texture) const;
	SDL_Texture* GetRenderTarget() const;
	void RenderCopy(GUITexture* texture, GUIPoint topleft) const;
	void RenderCopy(GUITexture* texture, GUIRect dstrect) const;
	void RenderCopy(GUITexture* texture, GUIPoint topleft, double angle, GUIPoint pointToRotate) const;
	void RenderCopy(GUITexture* texture, GUIRect dstrect, double angle, GUIPoint pointToRotate) const;
	SDL_RendererInfo GetInfo() const;
    GUITexture* LoadTextureImageData(SDL_RWops* rwops);
    
	GUITexture* CreateTextureFromSurface(SDL_Surface* surface) const;
	GUITexture* RenderTextBlended(TTF_Font* font, std::string text, SDL_Color foregroundColor) const;
	GUITexture* RenderTextBlendedWrapped(TTF_Font* font, std::string text, SDL_Color foregroundColor,Uint32 wrapLength) const;
	
	//DrawFuktions
	void DrawColor(SDL_Color color) const;
	void DrawColor(Uint32 color) const;
	void DrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) const;
	void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color) const;
	void DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) const;
	void DrawPixel(int x, int y, Uint32 color) const;
	void DrawPixel(int x, int y, SDL_Color color) const;
	void DrawPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) const;
	void DrawMidpointEllipse(int centerX, int centerY, int width, int height, SDL_Color color, bool filled, int lineWidth) const;
	GUITexture* LoadImageTexture(std::string fielName) const;
	void DrawFillRect(GUIRect rect) const;
	void DrawRoundFillRect(GUIRect rect, int radius) const;
	void DrawFillRect(GUIRect rect, SDL_Color color) const;
	void DrawRoundFillRect(GUIRect rect, int radius, SDL_Color color) const;
	void DrawRect(GUIRect rect) const;
	void DrawRoundRect(GUIRect rect, int radius) const;
	void DrawRect(GUIRect rect, SDL_Color color) const;
	void DrawRoundRect(GUIRect rect, int radius, SDL_Color color) const;
};
