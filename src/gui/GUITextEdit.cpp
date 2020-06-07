#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUITextEdit"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include "../exception/TTFException.h"
#include "GUI.h"
#include "GUIElement.h"
#include "GUITextEdit.h"
#include "GUIRenderer.h"
#include "GUITexture.h"
#include "GUIFontManager.h"

GUITextEdit::GUITextEdit(GUIPoint position, GUISize size, SDL_Color background, const std::string& name):
    GUIElement(position, size, name) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = background;
    text_ = "";
    textureText_ = nullptr;
    cursorOn_ = false;
    font_ = nullptr;
    lastCursorTick_ = 0;
    _OnFocus = nullptr;
    buttonDownEvent_ = std::bind(&GUITextEdit::ButtonDown, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

GUITextEdit::~GUITextEdit()
{
}

void GUITextEdit::Draw() {
    int move = 1;
    renderer_->DrawRect(GUIRect(0,0,Size()), own_blue_color);
    auto posCursor = 2;
    if(textureText_ != nullptr) {
        int posAfterText = textureText_->Size().width;
        if(textureText_->Size().width > Size().width) {
            int zeichenCount = 2;
            int h = 0;
            do
            {
                posAfterText = textureText_->Size().width;
                std::string lastZeichen = text_.substr(text_.size() - zeichenCount, zeichenCount);
                if(TTF_SizeUTF8(font_, lastZeichen.c_str(), &move, &h) != 0) {
                    throw TTFException("SizeUTF8");
                }
                posAfterText -= move;
                zeichenCount += 1;
            } while (posAfterText > Size().width);
            move = -move;
        }
        posCursor = posAfterText + 2; 
    }
	if(cursorOn_) {
        renderer_->DrawLine(posCursor,2,posCursor,Size().height-4, own_blue_color);
    } else {
        renderer_->DrawLine(posCursor,2,posCursor,Size().height-4, backgroundColor_);
    }
    if(textureText_ != nullptr) {
        renderer_->RenderCopy(textureText_, GUIPoint(move, 1));
    }
	needRedraw_ = false;
}

void GUITextEdit::HandleEvent(GUIEvent& event) {
    if(!selected_) return;

    bool renderText = false;
    switch (event.Type) {
         case SDL_KEYDOWN:{
            if( event.Event.key.keysym.sym == SDLK_BACKSPACE && text_.length() > 0 )
            {
                do {
                    if ((text_[text_.length()-1] & 0x80) == 0x00)
                    {
                        /* One byte */
                        text_.pop_back();
                        break;
                    }
                    if ((text_[text_.length()-1] & 0xC0) == 0x80)
                    {
                        /* Byte from the multibyte sequence */
                        text_.pop_back();
                    }
                    if ((text_[text_.length()-1] & 0xC0) == 0xC0)
                    {
                        /* First byte of multibyte sequence */
                        text_.pop_back();
                        break;
                    }
                } while(1);
                renderText = true;
            }
            else if( event.Event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
            {
                SDL_SetClipboardText(text_.c_str() );
            }
            else if( event.Event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
            {
                text_ = SDL_GetClipboardText();
                renderText = true;
            }
            break;
        }
        
        case SDL_TEXTINPUT:{
            //Not copy or pasting
            if( !( ( event.Event.text.text[ 0 ] == 'c' || event.Event.text.text[ 0 ] == 'C' ) && (event.Event.text.text[ 0 ] == 'v' || event.Event.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL ) )
            {
                //Append character
                //VLOG(5) << "Text Input " << event.Event.text.text << " " << cursorPos_;
                text_ += event.Event.text.text;
                renderText = true;
            }
            break;
        }
    }
    
    if(renderText) {
        RenderText();
        needRedraw_ = true;
    }
}

void GUITextEdit::UpdateAnimation()
{
    if(!selected_) return;

    if(!SDL_TICKS_PASSED(SDL_GetTicks(), lastCursorTick_ + 800)){
        return;
    }
    cursorOn_ =! cursorOn_;
    lastCursorTick_ = SDL_GetTicks();
    needRedraw_ = true;
}

void GUITextEdit::Close() {
    if(textureText_ != nullptr) {
		delete textureText_;
		textureText_ = nullptr;
	}
    
    SDL_StopTextInput();
}

void GUITextEdit::Init() {
    //Things after Control is Created
    font_ = fontManager_->GetDefaultSmallFont(Size().height - 4);
    RenderText();
}

void GUITextEdit::RenderText() {
    if (textureText_ != nullptr) delete textureText_;
    textureText_ = renderer_->RenderTextBlended(font_, text_, own_blue_color);
}

void GUITextEdit::ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point) {
    if(_OnFocus != nullptr) {
        _OnFocus(this);
    }
}

void GUITextEdit::Select() {
    cursorOn_ = true;
    GUIElement::Select();
    SDL_StartTextInput();
    GUIRect rect(0,0,Size());
    SDL_SetTextInputRect(&rect);
}

void GUITextEdit::Unselect() {
    cursorOn_ = false;
    GUIElement::Unselect();
    SDL_StopTextInput();
}

/**
 * \brief Get the Text from Editfield
 * \return the Text
 */
std::string GUITextEdit::GetText() {
  return text_;
}

void GUITextEdit::SetText(const std::string& text) {
    text_ = text;
    RenderText();
    needRedraw_ = true;
}

void GUITextEdit::RegisterOnFocus(FocusDelegate OnFocus) {
    _OnFocus = OnFocus;
}