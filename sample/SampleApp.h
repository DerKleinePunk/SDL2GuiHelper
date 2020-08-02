#pragma once

#include "../src/MiniKernel.h"
#include "../src/gui/GUIElementManager.h"

class SampleApp
{
private:
    MiniKernel* _kernel;
    GUIElementManager* _manager;
    void ApplicationEvent(AppEvent event, void* data1, void* data2);
    void KernelstateChanged(KernelState state);
    void BuildFirstScreen();
    void LoadSdsList();
    void SendRadioState(int state);
public:
    SampleApp(MiniKernel* kernel);
    virtual ~SampleApp();

    void Startup();
    void Shutdown();
};

