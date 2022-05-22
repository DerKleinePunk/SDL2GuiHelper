#pragma once
#include "../../src/Elements.h"

class MusikPlayerDialog
{
private:
    GUIElement* _parent;
    GUIElementManager* _manager;
    SDLEventManager* _eventManager;
    std::string _currentMusikFile;
public:
    MusikPlayerDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager);
    virtual ~MusikPlayerDialog();

    void Init();
    void Show();
    void Hide();

    void PlayMusik();
};
