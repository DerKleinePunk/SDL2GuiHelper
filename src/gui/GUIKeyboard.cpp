#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIKeyboard"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"


//https://github.com/postmarketOS/osk-sdl/tree/master/src
//https://github.com/postmarketOS/osk-sdl/blob/612138168dea2aa8300c374a2eb0446ce08cb876/src/util.cpp

#include <string>
#include "GUI.h"
#include "GUIElement.h"
#include "GUIKeyboard.h"
#include "GUIRenderer.h"
#include "GUIFontManager.h"
#include "GUITexture.h"

#define FONT_HEIGHT 20

void GUIKeyboard::LoadKeymap() {
    KeyboardLayer layer0, layer1, layer2;
    keyboard_.clear();

    layer0.row1 = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
    layer0.row2 = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"};
    layer0.row3 = {"a", "s", "d", "f", "g", "h", "j", "k", "l"};
    layer0.row4 = {KEYCAP_SHIFT, "z", "x", "c", "v", "b", "n", "m", KEYCAP_BACKSPACE};
    layer0.layerNum = 0;
    layer0.texture = nullptr;

    layer1.row1 = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"};
    layer1.row2 = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
    layer1.row3 = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
    layer1.row4 = {KEYCAP_SHIFT, "Z", "X", "C", "V", "B", "N", "M", KEYCAP_BACKSPACE};
    layer1.layerNum = 1;
    layer1.texture = nullptr;

    layer2.row1 = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"};
    layer2.row2 = {";", ":", "'", "\"", ",", ".", "<", ">", "/", "?"};
    layer2.row3 = {"-", "_", "=", "+", "[", "]", "{", "}", "\\"};
    layer2.row4 = {KEYCAP_SHIFT, "|", "\u20a4", "\u20ac", "\u2211", "\u221e", "\u221a", "\u2248", KEYCAP_BACKSPACE};
    layer2.layerNum = 2;
    layer2.texture = nullptr;

    keyboard_.push_back(layer0);
    keyboard_.push_back(layer1);
    keyboard_.push_back(layer2);
}

GUIKeyboard::GUIKeyboard(GUIPoint position, GUISize size, SDL_Color background, const std::string& name):
    GUIElement(position, size, name) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = background;
    font_ = nullptr;
    currentLayer_ = 0;
    buttonDownEvent_ = std::bind(&GUIKeyboard::ButtonDown, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    buttonUpEvent_ = std::bind(&GUIKeyboard::ButtonUp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

GUIKeyboard::~GUIKeyboard()
{
}

void GUIKeyboard::Init() {
    //Things after Control is Created
    font_ = fontManager_->GetDefaultSmallFont(FONT_HEIGHT);
    LoadKeymap();
    for (auto layer = keyboard_.begin(); layer != keyboard_.end(); ++layer) {
        layer->texture = renderer_->CreateTexture(Size());
        DrawLayer(&(*layer));
    }
    renderer_->RenderTarget(nullptr);
}

void GUIKeyboard::DrawLayer(KeyboardLayer* layer) {
    renderer_->RenderTarget(layer->texture);
    renderer_->Clear(lightgray_t_color);
    
    int rowHeight = Size().height / 5;
    
    DrawRow(&layer->keyList, 0, 0, Size().width / 10, rowHeight, &layer->row1, Size().width / 100, font_);
    DrawRow(&layer->keyList, 0, rowHeight, Size().width / 10, rowHeight, &layer->row2, Size().width / 100, font_);
    DrawRow(&layer->keyList, Size().width / 20, rowHeight * 2, Size().width / 10, rowHeight, &layer->row3, Size().width / 100, font_);
    DrawRow(&layer->keyList, Size().width / 20, rowHeight * 3, Size().width / 10, rowHeight, &layer->row4, Size().width / 100, font_);
    
    // Divide the bottom row in 20 columns and use that for calculations
    int colw = Size().width / 20;

    /* Draw symbols or ABC key based on which layer we are creating */
    if (layer->layerNum < 2) {
      char symb[] = "=\\<";
      DrawKey(&layer->keyList, colw, rowHeight * 4, colw*3, rowHeight, symb, &KEYCAP_SYMBOLS, Size().width / 100, font_);
    }else if (layer->layerNum == 2) {
      char abc[] = "abc";
      DrawKey(&layer->keyList, colw, rowHeight * 4, colw*3, rowHeight, abc, &KEYCAP_ABC, Size().width / 100, font_);
    }

    char space[] = " ";
    DrawKey(&layer->keyList, colw*5, rowHeight * 4, colw*10, rowHeight, space, &KEYCAP_SPACE, Size().width / 100, font_);

    /*char enter[] = "OK";
    DrawKey(&layer->keyList, colw*15, rowHeight * 4,  colw*5, rowHeight, enter, &KEYCAP_RETURN, Size().width / 100, font_);
     */
}

void GUIKeyboard::DrawRow(std::vector<touchArea> *keyList, int x, int y, int width, int height, std::list<std::string> *keys, int padding, TTF_Font *font) {
    auto i = 0;
    for (auto keyCap = keys->begin(); keyCap != keys->end(); ++keyCap) {
        auto buttonX = x + (i * width);
        DrawKey(keyList, buttonX, y, width, height, keyCap->c_str(), &*keyCap, padding, font);
        i++;
    }
}

void GUIKeyboard::DrawKey(std::vector<touchArea> *keyList, int x, int y, int width, int height, const char *cap, const std::string* key,int padding, TTF_Font *font) {
    GUIRect keyRect;
    keyRect.x = x + padding;
    keyRect.y = y + padding;
    keyRect.w = width - (2 * padding);
    keyRect.h = height - (2 * padding);
    renderer_->DrawFillRect(keyRect, own_blue_color);

    keyList->push_back({*key, GUIRect(x, y, width, height)});

    const auto textureText = renderer_->RenderTextBlended(font_, cap, black_color);
    const auto point = GUIPoint(keyRect.x + ((keyRect.w / 2) - (textureText->Size().width / 2)), keyRect.y + ((keyRect.h / 2) - (textureText->Size().height / 2)));
    renderer_->RenderCopy(textureText, point);
    delete textureText;
}

std::string GUIKeyboard::GetKeyFromPoint(const GUIPoint& point) {
    for (auto layer = keyboard_.begin(); layer != keyboard_.end(); ++layer) {
        if ((*layer).layerNum == currentLayer_) {
            for (auto it = (*layer).keyList.begin(); it != (*layer).keyList.end(); ++it) {
                if(SDL_PointInRect(&point, &it->rect)) {
                    return it->keyChar;
                }
            }
            break;
        }
    }
    return "";
}

void GUIKeyboard::ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point) {
    const auto pointInElement = ScreenToElementCoords(point);
    if (button == SDL_BUTTON_LEFT) {
        auto key = GetKeyFromPoint(pointInElement);
        if(!key.empty()) {
            //Todo Color Key Button
            lastKeyDown_ = key;
        }
    }
}

void GUIKeyboard::ButtonUp(Uint8 button, Uint8 clicks, const GUIPoint& point) {
    const auto pointInElement = ScreenToElementCoords(point);
    if (button == SDL_BUTTON_LEFT) {
        auto key = GetKeyFromPoint(pointInElement);
        if(!key.empty()) {
            //Todo Color Key Button
            if(lastKeyDown_ == key){
                if(key.compare(KEYCAP_SHIFT) == 0) {
                    if(currentLayer_ == 0){
                        currentLayer_ = 1;
                    } else if(currentLayer_ == 1){
                        currentLayer_ = 0;
                    }
                    SetRedraw();
                }
                else if(key.compare(KEYCAP_SYMBOLS) == 0) {
                    currentLayer_ = 2;
                    SetRedraw();
                }
                else if (key.compare(KEYCAP_ABC) == 0){
                    currentLayer_ = 0;
                    SetRedraw();
                }
                else if(key.compare(KEYCAP_BACKSPACE) == 0) {
                    //Todo
                    SDL_Event event;
                    event.key.type = SDL_KEYDOWN;
                    event.key.windowID = windowId_;
                    event.key.keysym.sym = SDLK_BACKSPACE;
                    SDL_PushEvent(&event);
                } else {
                    SDL_Event event;
                    event.text.type = SDL_TEXTINPUT;
                    event.text.windowID = windowId_;
                    strncpy(event.text.text, key.c_str(), 31);
                    SDL_PushEvent(&event);
                }
            }
            lastKeyDown_ = "";
        }
    }
}

void GUIKeyboard::Draw() {
    for (auto layer = keyboard_.begin(); layer != keyboard_.end(); ++layer) {
        if ((*layer).layerNum == currentLayer_) {
          renderer_->RenderCopy((*layer).texture, GUIPoint(0,0));
          break;
        }
    }
    needRedraw_ = false;
}

void GUIKeyboard::HandleEvent(GUIEvent& event) {
    UNUSED(event);
}

void GUIKeyboard::UpdateAnimation() {
    
}

void GUIKeyboard::Close() {
    for (auto layer = keyboard_.begin(); layer != keyboard_.end(); ++layer) {
        delete layer->texture;
    }
}