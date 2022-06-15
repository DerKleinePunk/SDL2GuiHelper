#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "AutomationDialog"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "AutomationDialog.h"

#include "../../src/gui/GUIElementManager.h"
#include "SampleApp.h"

AutomationDialog::AutomationDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager)
{
    _parent = parent;
    _manager = manager;
    _eventManager = eventManager;
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

AutomationDialog::~AutomationDialog()
{
}

void AutomationDialog::Init()
{
    _progressbar = new GUIProgressbar(GUIPoint(0, 45), GUISize(100, 50,sizeType::relative), "batterieLevel", own_firered_color, white_color);
    _manager->AddElement(_parent, _progressbar);

    _gauge = new GUIGauge(GUIPoint(0, 45), GUISize(100, 50,sizeType::relative), "batterieLevel2", own_firered_color, white_color);
    _manager->AddElement(_parent, _gauge);

    auto zoomDownButton = new GUITextButton(GUIPoint(20, 365), GUISize(300, 60), "value25Button", own_firered_color, white_color);
    zoomDownButton->Text("25");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { SetValue(25); });
    _manager->AddElement(_parent, zoomDownButton);

    zoomDownButton = new GUITextButton(GUIPoint(20, 430), GUISize(300, 60), "value50Button", own_firered_color, white_color);
    zoomDownButton->Text("50");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { SetValue(50); });
    _manager->AddElement(_parent, zoomDownButton);

    zoomDownButton = new GUITextButton(GUIPoint(325, 365), GUISize(300, 60), "value75Button", own_firered_color, white_color);
    zoomDownButton->Text("75");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { SetValue(75); });
    _manager->AddElement(_parent, zoomDownButton);

    zoomDownButton = new GUITextButton(GUIPoint(325, 430), GUISize(300, 60), "value100Button", own_firered_color, white_color);
    zoomDownButton->Text("100");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { SetValue(100); });
    _manager->AddElement(_parent, zoomDownButton);

    zoomDownButton = new GUITextButton(GUIPoint(650, 365), GUISize(300, 60), "switchButton", own_firered_color, white_color);
    zoomDownButton->Text("Switch");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { SwitchGui(); });
    _manager->AddElement(_parent, zoomDownButton);

    _progressbar->Invisible();
    _progressbarShow = false;
}

void AutomationDialog::Show()
{
    _parent->Visible();
}

void AutomationDialog::Hide()
{
    _parent->Invisible();
}

void AutomationDialog::SetValue(unsigned char value)
{
    _progressbar->SetValue(value);
    _gauge->SetValue(value);
}

void AutomationDialog::SwitchGui()
{
     if(_progressbarShow) {
        _progressbar->Invisible();
        _gauge->Visible();
    } else {
        _progressbar->Visible();
        _gauge->Invisible();
    }

    _progressbarShow = !_progressbarShow;
}