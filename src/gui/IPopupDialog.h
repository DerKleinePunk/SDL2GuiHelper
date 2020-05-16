#pragma once
#include <functional>

class GUIElementManager;
class GUIElement;

typedef std::function<void()> CloseDelegate;

class IPopupDialog
{
protected:
    GUIElement* screenElement_;
    GUIElementManager* manager_;
    CloseDelegate closeAction_;
    
    virtual void CreateIntern() = 0;
public:
    IPopupDialog(GUIElementManager* manager);
    virtual ~IPopupDialog();

    GUIElement* Create(CloseDelegate closeAction);
    void Close();
};

