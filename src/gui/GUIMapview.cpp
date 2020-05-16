#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIMapview"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUIMapview.h"
#include "GUIFontManager.h"
#include "GUITexture.h"
#include "GUIRenderer.h"
#include "../SDLEventManager.h"

#ifdef LIBOSMSCOUT
    #include "../map/MapManager.h"
#endif

#define REDRAW 1
#define BREAK 2
#define UPDATEADDDATA 3

void GUIMapview::HandleMapEvent(GUIEvent& event) {
    if(event.Event.user.code == REDRAW) {
        LOG(DEBUG) << "MapEvent Redraw";
        if(mapTexture_ == nullptr){
            mapTexture_ = renderer_->CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GUISize(Size().width * 2, Size().height * 2));
        }
        void *pixels;
        int pitch;
        SDL_LockMutex(mapMemLock_);
        if(SDL_LockTexture(mapTexture_->operator SDL_Texture*(), NULL, &pixels, &pitch) == 0) {
            memcpy(pixels, mapPixels_, pitch * Size().height * 2);
            SDL_UnlockTexture(mapTexture_->operator SDL_Texture*());
            SetRedraw();
            initOk_ = true;
        } else {
            LOG(ERROR) << SDL_GetError();
        }
        SDL_UnlockMutex(mapMemLock_);
    }
    
    if(event.Event.user.code == UPDATEADDDATA) {
        LOG(DEBUG) << "MapEvent UpdateData";
        auto textureText = renderer_->RenderTextBlended(font_, currentStreetName_, black_color);
        if(headTexture_ != nullptr) {
            delete headTexture_;
        }
        headTexture_ = renderer_->CreateTexture(GUISize(textureText->Size().width + 20,26));
        renderer_->RenderTarget(headTexture_);
        auto color = own_blue_color;
        color.a = 40;
        renderer_->Clear(color);
        renderer_->RenderCopy(textureText,GUIPoint(10,0));
        delete textureText;
        
        if(statusbarTexture_ == nullptr) {
            statusbarTexture_ = renderer_->CreateTexture(GUISize(300,40));
        }
        renderer_->RenderTarget(statusbarTexture_);
        renderer_->Clear(lightgray_t_color);
        
        std::string speedText;
        if(currentMaxSpeed_ > -1) {
            speedText = std::to_string(currentSpeed_) + "/" + std::to_string(currentMaxSpeed_);
        } else {
            speedText = std::to_string(currentSpeed_);
        }
        textureText = renderer_->RenderTextBlended(font_, speedText, black_color);
        renderer_->RenderCopy(textureText,GUIPoint(10,0));
        
        SetRedraw();
    }
}

void GUIMapview::RenderMap() {
    
}

void GUIMapview::InitMap(){
#ifdef OSMSCOUT
    auto newMapPixeldelegate = std::bind(&GUIMapview::NewMapPixel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto newNamedelegate = std::bind(&GUIMapview::NewMapNameOrSpeed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    if(mapManager_ == nullptr) {
        LOG(ERROR) << "mapManager_ == nullptr";
        return;
    }
    mapManager_->RegisterMe(Size().width, Size().height, newMapPixeldelegate, newNamedelegate);
#endif
}

GUIMapview::GUIMapview(const GUIPoint position, const GUISize size, const std::string& name, const SDL_Color background, const SDL_Color textcolor) :
	GUIElement(position, size, name), 
    font_(nullptr),
    mapPixels_(nullptr) {
    logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    backgroundColor_ = background;
    textcolor_ = textcolor;
    initOk_ = false;
    mapTexture_ = nullptr;
    mapMemLock_ = SDL_CreateMutex();
    viewDeltaNow_.x = 0;
    viewDeltaNow_.y = 0;
    streetImageTexture_ = nullptr;
    headTexture_ = nullptr;
    statusbarTexture_ = nullptr;
}

void GUIMapview::Init() {
    //Things after Control is Created
    font_ = fontManager_->GetDefaultSmallFont(24);
    mapEvent_ = SDL_RegisterEvents(1);
    InitMap();
}

void GUIMapview::Draw() {
    if(initOk_ && mapTexture_){
        GUIPoint drawPos;
        drawPos.x = viewDeltaNow_.x - (mapTexture_->Size().width / 4);
        drawPos.y = viewDeltaNow_.y - (mapTexture_->Size().height / 4);
        renderer_->RenderCopy(mapTexture_, drawPos);
        if(headTexture_) {
            renderer_->RenderCopy(headTexture_, GUIRect((Size().width / 2) - (headTexture_->Size().width / 2), 1, headTexture_->Size()));
        }
        if(statusbarTexture_ != nullptr) {
            renderer_->RenderCopy(statusbarTexture_, GUIRect((Size().width / 2) - (statusbarTexture_->Size().width / 2), 600 - (statusbarTexture_->Size().height + 2), statusbarTexture_->Size()));
        }
    } else {
        auto textureText = renderer_->RenderTextBlended(font_, "Karte nicht bereit", textcolor_);
        renderer_->RenderCopy(textureText, GUIRect((Size().width / 2) - (textureText->Size().width / 2), Size().height / 2, textureText->Size()));
        delete textureText;
    }
    needRedraw_ = false;
}

void GUIMapview::HandleEvent(GUIEvent& event) {
	if(event.Type == mapEvent_) {
        HandleMapEvent(event);
    }
}

void GUIMapview::UpdateAnimation() {
}

void GUIMapview::Close() {

#ifdef LIBOSMSCOUT
    mapManager_->Unregister();
#endif
    
    if(mapPixels_ != nullptr) delete[] mapPixels_;
    
    SDL_DestroyMutex(mapMemLock_);
    
    if(mapTexture_ != nullptr) {
        delete mapTexture_;
    }
    
    if(statusbarTexture_ != nullptr) {
        delete statusbarTexture_;
    }
    
    if(headTexture_ != nullptr) {
        delete headTexture_;
    }
}

void GUIMapview::NewMapPixel(unsigned char* mapPixels, int mapWidth, int mapHeight) {
    //Becarfull called from Thread
    LOG(DEBUG) << "NewMapPixel";
    //Todo Size Change
    SDL_LockMutex(mapMemLock_);
    if(mapPixels_ == nullptr) mapPixels_ = new unsigned char[mapWidth * mapHeight * 4];
    memcpy(mapPixels_, mapPixels, mapWidth * mapHeight * 4);
    SDL_UnlockMutex(mapMemLock_);
    EventManager()->PushEvent(mapEvent_, windowId_, REDRAW, nullptr, nullptr);
}

void GUIMapview::NewMapNameOrSpeed(const std::string& name, const int& maxSpeed, const int& currentSpeed) {
    LOG(DEBUG) << "NewMapNameOrSpeed " << name << " " << maxSpeed << " " << currentSpeed;
    
    if(currentStreetName_ == name && currentMaxSpeed_ == maxSpeed && currentSpeed_ == currentSpeed) {
        return;
    }
    
    currentMaxSpeed_ = maxSpeed;
    currentSpeed_ = currentSpeed;
    
    if(name.size() == 0){
        currentStreetName_ = "Strasse ohne Namen";
    } else {
        currentStreetName_ = name;
    }
    
    EventManager()->PushEvent(mapEvent_, windowId_, UPDATEADDDATA, nullptr, nullptr);
}