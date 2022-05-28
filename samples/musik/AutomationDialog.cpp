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
    _progressbar = new GUIProgressbar(GUIPoint(0, 0), GUISize(100, 50,sizeType::relative), "batterieLevel", own_firered_color, white_color);
    _manager->AddElement(_parent, _progressbar);
}

void AutomationDialog::Show()
{
    _parent->Visible();
}

void AutomationDialog::Hide()
{
    _parent->Invisible();
}