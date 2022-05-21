#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "ErrorMessageDialog"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../common/easylogging/easylogging++.h"
#include "ErrorMessageDialog.h"
#include "gui/GUI.h"
#include "gui/GUIElement.h"
#include "gui/GUIOnClickDecorator.h"
#include "gui/GUITestElement.h"
#include "gui/GUITextButton.h"
#include "gui/GUITextLabel.h"
#include "gui/GUIElementManager.h"
#include "SDLEventManager.h"
#include "AppEvents.h"

ErrorMessageDialog::ErrorMessageDialog(GUIElementManager* manager):
    IPopupDialog(manager) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

ErrorMessageDialog::~ErrorMessageDialog()
{
}

void ErrorMessageDialog::CreateIntern() {
    auto dialog = new GUITestElement(GUIPoint(112, 100), GUISize(800, 400), white_color, "ErrorScreen");
    dialog->SetBorder(true);

    screenElement_ = dialog;

    manager_->AddElement(screenElement_);
    
    auto errorTextLabel = new GUITextLabel(GUIPoint(30, 35), GUISize(740, 300), "errorTextLabel", lightgray_t_color, own_red_color);
	manager_->AddElement(screenElement_, errorTextLabel);
	errorTextLabel->FontHeight(22);
	errorTextLabel->Text(message_);
    
    auto test5 = new GUITextButton(GUIPoint(100, 335), GUISize(100, 50), "OkButton", own_blue_color, white_color);
	manager_->AddElement(screenElement_, test5);
	test5->Text("Ok");
	test5->RegisterOnClick([this](IGUIElement* sender) {
        sender->EventManager()->PushApplicationEvent(AppEvent::ClosePopup, this, nullptr); 
    });
}

void ErrorMessageDialog::SetMessage(const std::string& message) {
    message_ = message;
}