#pragma once
#include <string>
#include "gui/GUIElementManager.h"
#include "gui/IPopupDialog.h"

class ErrorMessageDialog: public IPopupDialog
{
    std::string message_;
    void CreateIntern();
public:
    ErrorMessageDialog(GUIElementManager* manager);
    ~ErrorMessageDialog();

    void SetMessage(const std::string& message);
};