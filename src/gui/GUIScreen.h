#pragma once
#include "GUI.h"
#include "IMapManager.h"

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

	bool HandleWindowEvent(const SDL_Event* event) const;
public:
	GUIScreen();
	virtual ~GUIScreen();

	GUIElementManager* Create(std::string title, SDLEventManager* eventManager, IMapManager* mapManager, const std::string& backgroundImage);
	Uint32 GetId() const;
	void UpdateAnimationInternal() const;
	void Draw() const;
	void Resize(Sint32 data1, Sint32 data2) const;
	void HandleEvent(const SDL_Event* event) const;
	bool NeedRedraw() const;
	void ToggleFullscreen();

	void Shutdown();
};
