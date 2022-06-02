#pragma once
#include <map>
#include "SDLBase.h"
#include "SDLEventManager.h"
#include "gui/GUIScreen.h"
#include "gui/GUIElementManager.h"
#include "KernelConfig.h"
#include "IAudioManager.h"

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
    std::vector<ApplicationEventCallbackFunction> _applicationEventCallbackFunctions;
    float _screenDpi;
    GUIElementManager* _manager;
    std::string _errorMessage;
    KernelConfig _kernelConfig;
    IAudioManager* _audioManager;
    IMapManager* _mapManager;

    void HandleEvent(const SDL_Event& event,bool& exitLoop);
    int StartAudio(const std::string& drivername);
    void UpdateScreens();
public:
    MiniKernel();   
    ~MiniKernel();

    bool StartUp(int argc, char* argv[]);
    void Run();
    void Shutdown();
    void StartCoreServices();
    GUIElementManager* CreateScreen(const std::string& title, const std::string& videoDriver, const std::string& backgroundImage, bool fullscreen);
    void SetStateCallBack(KernelStateCallbackFunction callback);
    void RegisterApplicationEvent(ApplicationEventCallbackFunction callbackFunction);
    void DrawTextOnBootScreen(const std::string& text);
    SDLEventManager* GetEventManager() const;
    void ShowErrorMessage(const std::string& message);
    int PlaySound(const std::string& filename) const;
    void SetConfig(const KernelConfig& config);
};

