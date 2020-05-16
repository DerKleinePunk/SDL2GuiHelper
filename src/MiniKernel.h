#pragma once
#include "SDLBase.h"
#include "SDLEventManager.h"
#include "gui/GUIScreen.h"
#include <map>

typedef std::function<void(KernelState state)> KernelStateCallbackFunction;
typedef std::function<void(AppEvent code, void* data1, void* data2)> ApplicationEventCallbackFunction;

class MiniKernel
{
private:
    SDLBase* _base;
    SDLEventManager* _eventManager;
    bool _firstrun;
    std::map<Uint32, GUIScreen*> _screens;
    KernelStateCallbackFunction _callbackState;
    ApplicationEventCallbackFunction _applicationEventCallbackFunction;
    float _screenDpi;
    void HandleEvent(const SDL_Event& event,bool& exitLoop);
public:
    MiniKernel();
    ~MiniKernel();

    bool StartUp(int argc, char* argv[]);
    void Run();
    void Shutdown();
    GUIElementManager* CreateScreen(const std::string& title, const std::string& videoDriver);
    void SetStateCallBack(KernelStateCallbackFunction callback);
    void RegisterApplicationEvent(ApplicationEventCallbackFunction callbackFunction);
};

