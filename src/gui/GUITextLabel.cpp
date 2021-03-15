#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "GUITextLabel"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "GUITextLabel.h"
#include "../../common/easylogging/easylogging++.h"
#include "../exception/TTFException.h"
#include "GUI.h"
#include "GUIElement.h"
#include "GUIFontManager.h"
#include "GUIOnClickDecorator.h"
#include "GUIRenderer.h"
#include "GUITexture.h"

void GUITextLabel::GetFont()
{
    if(!smallFont_) {
        font_ = fontManager_->GetDefaultFont(fontHeight_);
    } else {
        font_ = fontManager_->GetDefaultSmallFont(fontHeight_);
    }
}

GUITextLabel::GUITextLabel(const GUIPoint position,
                           const GUISize size,
                           const std::string& name,
                           const SDL_Color background,
                           const SDL_Color textcolor)
    : GUITextLabel(position, size, name, background, textcolor, false, own_red_color, background)
{
}

GUITextLabel::GUITextLabel(const GUIPoint position,
                           const GUISize size,
                           const std::string& name,
                           const SDL_Color background,
                           const SDL_Color textcolor,
                           const bool smallFont)
    : GUITextLabel(position, size, name, background, textcolor, smallFont, own_red_color, background)
{
}

GUITextLabel::GUITextLabel(const GUIPoint position,
                           const GUISize size,
                           const std::string& name,
                           const SDL_Color background,
                           const SDL_Color textcolor,
                           const bool smallFont,
                           const SDL_Color selectedColor,
                           const SDL_Color selectedBackgroundColor)
    : GUIElement(position, size, name), GUIOnClickDecorator(static_cast<GUIElement*>(this)),
      showTime_(false), smallFont_(smallFont), font_(nullptr), textureText_(nullptr),
      selectedColor_(selectedColor), selectedBackgroundColor_(selectedBackgroundColor),
      normalBackgroundColor_(background)
{
    backgroundColor_ = background;
    foregroundColor_ = textcolor;
    angle_ = 0;
    fontHeight_ = -1;
    textAnchor_ = AnchorFlags::Right;
    _corner = 0;
}

void GUITextLabel::Select()
{
    GUIElement::Select();
    if(_corner == 0) backgroundColor_ = selectedBackgroundColor_;
    RenderText();
}

void GUITextLabel::Unselect()
{
    GUIElement::Unselect();
    if(_corner == 0) backgroundColor_ = normalBackgroundColor_;
    RenderText();
}

void GUITextLabel::Text(const std::string& text)
{
    text_ = text;
    if(font_ != nullptr) {
        RenderText();
    }
    SetRedraw();
}

void GUITextLabel::ShowTime(bool on)
{
    showTime_ = on;
    needRedraw_ = true;
}

void GUITextLabel::Rotate(double angle)
{
    angle_ = angle;
    needRedraw_ = true;
}

void GUITextLabel::FontHeight(int fontHeight)
{
    fontHeight_ = fontHeight;
    if(font_ != nullptr) {
        GetFont();
        RenderText();
    }
    needRedraw_ = true;
}

void GUITextLabel::TextAnchor(AnchorFlags flags)
{
    textAnchor_ = flags;
}

void GUITextLabel::Init()
{
    // Things after Control is Created
    if(fontHeight_ == -1) {
        fontHeight_ = Size().height;
    }
    GetFont();
    RenderText();
}

void GUITextLabel::RenderText()
{
    if(textureText_ != nullptr) delete textureText_;

    auto textColor = foregroundColor_;
    if(selected_) textColor = selectedColor_;

    if(text_.find("\n") != std::string::npos) {
        textureText_ = renderer_->RenderTextBlendedWrapped(font_, text_, textColor, Size().width);
    } else {
        int w = 0;
        int h = 0;
        if(TTF_SizeUTF8(font_, text_.c_str(), &w, &h) != 0) {
            throw TTFException("SizeUTF8");
        }
        if(w <= Size().width) {
            textureText_ = renderer_->RenderTextBlended(font_, text_, textColor);
        } else {
            textureText_ = renderer_->RenderTextBlendedWrapped(font_, text_, textColor, Size().width);
        }
    }
    SetRedraw();
}

// http://stackoverflow.com/questions/3229391/sdl-sdl-ttf-transparent-blended-text
void GUITextLabel::Draw()
{
    if(_corner != 0) {
        if(selected_) {
            renderer_->DrawRoundFillRect(GUIRect(0, 0, Size().width, Size().height), GetCorner(),
                                         selectedBackgroundColor_);
        } else {
            renderer_->DrawRoundFillRect(GUIRect(0, 0, Size().width, Size().height), GetCorner(),
                                         normalBackgroundColor_);
        }
        renderer_->DrawRoundRect(GUIRect(0, 0, Size().width, Size().height), GetCorner(), foregroundColor_);
    }

    if(font_ == nullptr) {
        GetFont();
    }
    if(text_.size() > 0 && textureText_ == nullptr) {
        RenderText();
    }
    if(fontHeight_ > -1 && textureText_ != nullptr) {
        if(angle_ == 0) {
            if(textAnchor_ == AnchorFlags::Left) {
                renderer_->RenderCopy(textureText_, GUIPoint(Size().width - textureText_->Size().width, 0));
            } else if(textAnchor_ == (AnchorFlags::Left & AnchorFlags::Right)) {
                renderer_->RenderCopy(textureText_,
                                      GUIPoint((Size().width - textureText_->Size().width) / 2, 0));
            } else {
                renderer_->RenderCopy(textureText_, GUIPoint(5, 0));
            }
        } else {
            renderer_->RenderCopy(textureText_, GUIPoint(0, Size().height / 2), angle_,
                                  GUIPoint(textureText_->Size().width / 2, textureText_->Size().height / 2));
        }
    }

    needRedraw_ = false;
}

void GUITextLabel::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUITextLabel::UpdateAnimation()
{
    if(!showTime_) return;

    auto t = std::time(nullptr);
#ifdef _MSC_VER
    struct tm newtime;
    localtime_s(&newtime, &t);
#else
    auto newtime = *std::localtime(&t);
#endif
    std::stringstream newText;
    // newText << std::put_time(&newtime, "%c %Z");
    char szOut[255];
    auto size = strftime(szOut, 255, "%H:%M", &newtime);
    newText << std::string(szOut, size);
    if(newText.str() != text_) {
        Text(newText.str());
    }
}

void GUITextLabel::Close()
{
    if(textureText_ != nullptr) {
        delete textureText_;
        textureText_ = nullptr;
    }
}

int GUITextLabel::GetCorner() const
{
    return _corner;
}

void GUITextLabel::SetCorner(int corner)
{
    if(corner > 0) {
        backgroundColor_ = transparent_color;
    } else {
        if(selected_) {
            backgroundColor_ = selectedBackgroundColor_;
        } else {
            backgroundColor_ = normalBackgroundColor_;
        }
    }
    _corner = corner;
}

void GUITextLabel::ButtonDownUpdate(Uint8 button)
{
}

void GUITextLabel::ButtonUpUpdate(Uint8 button)
{
}