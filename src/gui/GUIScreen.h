#pragma once
#include "../IAudioManager.h"
#include "../IMapManager.h"
#include "GUI.h"
#include "../MiniKernel.h"

class GUIElement;
class GUIElementManager;
class GUIScreenCanvas;
class SDLEventManager;
class GUIImageManager;
class GUIRenderer;
class MiniKernel;

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
    MiniKernel* _kernel;

    int _displayWidth;
    int _displayHeight;
    int _lastDisplayIndex;

    bool HandleWindowEvent(const SDL_Event* event);
    void GetScreenSize();

  public:
    GUIScreen();
    virtual ~GUIScreen();

    GUIElementManager* Create(std::string title,
                              SDLEventManager* eventManager,
                              IMapManager* mapManager,
                              IAudioManager* audioManager,
                              MiniKernel* _kernel,
                              const std::string& backgroundImage,
                              bool fullscreen,
                              SDL_Color backgroundColor,
                              SDL_Color foregroundColor);
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
