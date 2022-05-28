#pragma once
#include "../../src/Elements.h"

class AutomationDialog
{
private:
    GUIElement* _parent;
    GUIElementManager* _manager;
    SDLEventManager* _eventManager;
    GUIProgressbar* _progressbar;
    
public:
    AutomationDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager);
    ~AutomationDialog();

    void Init();
    void Show();
    void Hide();

    void SetValue(unsigned char value);
};
