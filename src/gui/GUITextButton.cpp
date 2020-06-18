#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUITextButton"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUIFontManager.h"
#include "GUIRenderer.h"
#include "GUITextButton.h"
#include "GUITexture.h"
#include "IGUIElement.h"

void GUITextButton::GetFont()
{
    if(!smallFont_) {
        font_ = fontManager_->GetDefaultFont(fontHeight_);
    } else {
        font_ = fontManager_->GetDefaultSmallFont(fontHeight_);
    }
}

void GUITextButton::RenderText()
{
    if(textureText_ != nullptr) {
        delete textureText_;
        textureText_ = nullptr;
    }
    if(enabled_) {
        textureText_ = renderer_->RenderTextBlended(font_, text_, foregroundColor_);
    } else {
        //textureText_ = renderer_->RenderTextBlended(font_, text_, foregroundColor_);
    }
}

GUITextButton::GUITextButton(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor)
    : GUIElement(position, size, name), GUIOnClickDecorator(static_cast<GUIElement*>(this)), text_("Kein Text")
{
    logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = transparent_color;
    backgroundColorButton_ = background;
    //Todo put it on Config
    _disablebackgroundColor = lightgray_color;
    foregroundColor_ = textcolor;
    centertext_ = true;
    textureText_ = nullptr;
    font_ = nullptr;
    fontHeight_ = -1;
    smallFont_ = true;
    _corner = 0;
}

void GUITextButton::Text(const std::string& text)
{
    text_ = text;
    if(font_ != nullptr) {
        GetFont();
        RenderText();
    }
    needRedraw_ = true;
}

void GUITextButton::FontHeight(int fontHeight)
{
    fontHeight_ = fontHeight;
    if(font_ != nullptr) {
        GetFont();
        RenderText();
    }
    needRedraw_ = true;
}

void GUITextButton::Init()
{
    // Things after Control is Created
    if(fontHeight_ == -1) {
        fontHeight_ = Size().height - 2;
    }
    GetFont();
    RenderText();
    SetCorner(4);
}

void GUITextButton::Draw()
{
	if(_corner > 0) {
        if(enabled_) {
		    renderer_->DrawRoundFillRect(GUIRect(0, 0, Size().width, Size().height), GetCorner(), backgroundColorButton_);
        } else {
            renderer_->DrawRoundFillRect(GUIRect(0, 0, Size().width, Size().height), GetCorner(), _disablebackgroundColor);
        }
		renderer_->DrawRoundRect(GUIRect(1, 1, Size().width - 2, Size().height - 2), GetCorner(), foregroundColor_);
	}
    if(selected_) {
        renderer_->DrawRoundRect(GUIRect(2, 2, Size().width - 4, Size().height - 4), GetCorner(), foregroundColor_);
    }
    if(textureText_ != nullptr) {
        auto drawTextPosition = GUIPoint(0, 0);
        if(centertext_) {
            drawTextPosition.x = (Size().width - textureText_->Size().width) / 2;
            drawTextPosition.y = (Size().height - textureText_->Size().height) / 2;
        }
        renderer_->RenderCopy(textureText_, drawTextPosition);
    }

    needRedraw_ = false;
}

void GUITextButton::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUITextButton::UpdateAnimation()
{
}

void GUITextButton::Close()
{
    delete textureText_;
    textureText_ = nullptr;
}

void GUITextButton::ChangeBackColor(SDL_Color color)
{
	if(_corner > 0) {
		backgroundColorButton_ = color;
	} else {
    	backgroundColor_ = color;
	}
    SetRedraw();
}


int GUITextButton::GetCorner() const
{
    return _corner;
}

void GUITextButton::SetCorner(int corner)
{
    _corner = corner;
}

void GUITextButton::Disable() 
{
    GUIElement::Disable();
    if(font_ != nullptr) {
        RenderText();
        SetRedraw();
    }
}

void GUITextButton::Enable()
{
    GUIElement::Enable();
    if(font_ != nullptr) {
        RenderText();
        SetRedraw();
    }
}