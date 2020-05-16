#include "IPopupDialog.h"
#include "GUIElementManager.h"
#include "GUIElement.h"

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
    return screenElement_;
}

void IPopupDialog::Close() {
    if(closeAction_ != nullptr) {
        closeAction_();
    }
    
    if(screenElement_ != nullptr) {
        manager_->Close(screenElement_);
        screenElement_ = nullptr;
    }
}