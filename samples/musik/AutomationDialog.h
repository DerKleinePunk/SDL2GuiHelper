#pragma once
#include "../../src/Elements.h"

class AutomationDialog
{
private:
    GUIElement* _parent;
    GUIElementManager* _manager;
    SDLEventManager* _eventManager;
    GUIProgressbar* _progressbar;
    GUIGauge* _gauge;
    GUIArtificialHorizon* _horizon;
    bool _progressbarShow;
public:
    AutomationDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager);
    ~AutomationDialog();

    void Init();
    void Show();
    void Hide();

    void SetValue(unsigned char value);
    void SwitchGui();
};
