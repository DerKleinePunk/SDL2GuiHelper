#pragma once

#include "../../src/MiniKernel.h"
#include "../../src/gui/GUIElementManager.h"

#include "MapDialog.h"

// Todo Add it to Config
const SDL_Color own_firered_color = { 0xA0, 0x00, 0x1C, 0xFF };
const SDL_Color own_selected_command_color = { 0x28, 0x5B, 0x78, 0xFF };

enum class UiState : unsigned char {
	undefined,
	home,
	map,
	sds,
    musikPlayer
};

class SampleApp
{
private:
    MiniKernel* _kernel;
    GUIElementManager* _manager;
    MapDialog* _mapDialog;
    UiState _appUiStateCurrent; 

    void ApplicationEvent(AppEvent event, void* data1, void* data2);
    void KernelstateChanged(KernelState state);
    void BuildFirstScreen();
    void LoadSdsList();
    void SendRadioState(int state);
    void BuildMainScreen();
    void BuildMapScreen();
    void BuildCommandBar();
    void UpdateUI(UiState newUIState);
public:
    SampleApp(MiniKernel* kernel);
    virtual ~SampleApp();

    void Startup();
    void Shutdown();
};

