#include "IPopupDialog.h"
#include "GUIElementManager.h"
#include "GUIElement.h"


void IPopupDialog::HandleEvent(GUIEvent& event)
{
    if(screenElement_ == nullptr) return;
    screenElement_->HandleEvent(event);
}

IPopupDialog::IPopupDialog(GUIElementManager* manager):
    closeAction_(nullptr) {
    manager_ = manager;
    screenElement_ = nullptr;
}

IPopupDialog::~IPopupDialog() {
}

GUIElement* IPopupDialog::Create(CloseDelegate closeAction) {
    closeAction_ = closeAction;
    CreateIntern();
    manager_->SetModalElement(screenElement_);
    return screenElement_;
}

void IPopupDialog::Close() {
    manager_->SetModalElement(nullptr);
    if(closeAction_ != nullptr) {
        closeAction_();
    }
    
    if(screenElement_ != nullptr) {
        manager_->Close(screenElement_);
        screenElement_ = nullptr;
    }
}