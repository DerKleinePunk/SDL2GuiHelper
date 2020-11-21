#pragma once
#include "GUI.h"
#include "../IMapManager.h"

class GUIElement;
class GUIElementManager;
class GUIScreenCanvas;
class SDLEventManager;
class GUIImageManager;
class GUIRenderer;

class GUIScreen
{
	SDL_Window* window_;
	GUIRenderer* renderer_;
	GUIElementManager* manager_;
	GUIImageManager* imageManager_;

	Uint32 id_;
	GUIScreenCanvas* canvas_;
	GUISize size_;
	SDLEventManager* eventManager_;
	int _displayWidth;
	int _displayHeight;
	int _lastDisplayIndex;

	bool HandleWindowEvent(const SDL_Event* event);
	void GetScreenSize();
public:
	GUIScreen();
	virtual ~GUIScreen();

	GUIElementManager* Create(std::string title, SDLEventManager* eventManager, IMapManager* mapManager, const std::string& backgroundImage, bool fullscreen);
	Uint32 GetId() const;
	void UpdateAnimationInternal() const;
	void Draw() const;
	void Resize(Sint32 data1, Sint32 data2) const;
	void HandleEvent(const SDL_Event* event);
	bool NeedRedraw() const;
	void ToggleFullscreen();
	void DrawTextOnBootScreen(const std::string& text);
	
	void Shutdown();
};
