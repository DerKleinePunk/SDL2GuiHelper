#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MiniKernel"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MiniKernel.h"
#include <AppEvents.h>
#include "../common/easylogging/easylogging++.h"
#include "../common/exception/FileNotFoundException.h"
#include "../common/exception/IllegalStateException.h"
#include "../common/exception/NullPointerException.h"
#include "../common/utils/commonutils.h"
#include "ErrorMessageDialog.h"
#include "exception/GUIException.h"
#include "exception/SDLException.h"
#include "exception/TTFException.h"
#include "gui/GUIScreen.h"
#ifdef LIBOSMSCOUT
#include "map/MapManager.h"
#endif

//#define MILLESECONDS_PER_FRAME 1000.0/120.0       /* about 120 frames per second */
#define MILLESECONDS_PER_FRAME 1000.0 / 60.0 /* about 60 frames per second */

void MiniKernel::HandleEvent(const SDL_Event& event, bool& exitLoop)
{
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerBlkObjPresent, "HandleEvent", VLOG_IS_ON(4));
#endif

    GUIScreen* screen = nullptr;

    if(event.type != SDL_QUIT) {
        auto winId = event.window.windowID;
        if(winId == 0) {
            winId = event.user.windowID;
        }

        if(event.type == SDL_FINGERMOTION || event.type == SDL_FINGERDOWN || event.type == SDL_FINGERUP) {
            // Todo how to map touchId to Window
            LOG(DEBUG) << "Finger Event on Touch Id " << event.tfinger.touchId << " Event " << event.type;
            winId = _screens.begin()->first;
        }

        if(_screens.find(winId) != _screens.end()) {
            screen = _screens[winId];
        }

        if(screen) {
            screen->HandleEvent(&event);
        }

        KernelEvent type;
        if(_eventManager->IsKernelEvent(&event, type)) {
            switch(type) {
            case KernelEvent::Shutdown: {
                LOG(INFO) << "Kernel Event Shutdown";
                exitLoop = true;
                break;
            }
            case KernelEvent::ShowError: {
                LOG(INFO) << "Kernel Event UpdateScreen";
                auto errorMessageDialog = new ErrorMessageDialog(_manager);
                errorMessageDialog->SetMessage(_errorMessage);
                errorMessageDialog->Create(nullptr);
                break;
            }
            default: {
                LOG(WARNING) << "Not Implemented Kernel Event " << type;
            }
            }
        }

        AppEvent code;
        void* data1;
        void* data2;
        if(_eventManager->IsApplicationEvent(&event, code, data1, data2)) {
            // TODO better move to Kernel event ?
            // TODO better move Interface ?
            /*if(code == AppEvent::NewGeopos && mapManager_ != nullptr) {
                KernelGPSMessage* message = (KernelGPSMessage*)data1;
                configManager_->UpdateLastPosition(message->coord);
                mapManager_->CenterMap(message->coord, message->compass, message->speed);
            }*/
            if(code == AppEvent::ClosePopup) {
                const auto element = static_cast<IPopupDialog*>(data1);
                if(element != nullptr) {
                    element->Close();
                    delete element;
                }
                _applicationEventCallbackFunction(code, nullptr, data2);
            } else if(code == AppEvent::Click) {
                if(_kernelConfig.AudioFileForClick.length() > 0) {
                    if(PlaySound(_kernelConfig.AudioFileForClick) < 0) {
                        LOG(ERROR) << "Fehler beim Abspielen von Sound " << _kernelConfig.AudioFileForClick;
                    }
                }
            } else if(code == AppEvent::LongClick) {
                if(_kernelConfig.AudioFileForLongClick.length() > 0) {
                    if(PlaySound(_kernelConfig.AudioFileForLongClick) < 0) {
                        LOG(ERROR) << "Fehler beim Abspielen von Sound " << _kernelConfig.AudioFileForLongClick;
                    }
                }
            } else if(_applicationEventCallbackFunction != nullptr) {
                _applicationEventCallbackFunction(code, data1, data2);
            }
        }
    }

    switch(event.type) {
    case SDL_QUIT:
        exitLoop = true;
        break;
    case SDL_KEYDOWN: {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        switch(event.key.keysym.sym) {
        case SDLK_ESCAPE:
            exitLoop = true;
            break;
        case SDLK_q:
            if(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) {
                exitLoop = true;
            }
            break;
        case SDLK_f:
            if(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) {
                if(screen) {
                    screen->ToggleFullscreen();
                }
            }
            break;
        }
    }
    default:
        // LOG(DEBUG) << event.type << " unhandelt event type";
        break;
    }
}

MiniKernel::MiniKernel()
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _base = nullptr;
    _eventManager = nullptr;
    _firstrun = true;
    _callbackState = nullptr;
    _screenDpi = 0.0;
    _applicationEventCallbackFunction = nullptr;
    _mapManager = nullptr;
    _audioManager = nullptr;
}

MiniKernel::~MiniKernel()
{
}

bool MiniKernel::StartUp(int argc, char* argv[])
{
    LOG(INFO) << "Kernel is starting";

    _base = new SDLBase();
    _base->Init();

    _eventManager = new SDLEventManager();
    const auto result = _eventManager->Init();
    if(!result) {
        delete _base;
        delete _eventManager;
        return result;
    }
    return result;
}

void MiniKernel::UpdateScreens()
{
    auto screenPtr = _screens.begin();
    while(screenPtr != _screens.end()) {
        screenPtr->second->UpdateAnimationInternal();
        if(screenPtr->second->NeedRedraw()) {
            screenPtr->second->Draw();
        }
        ++screenPtr;
    }
}

void MiniKernel::Run()
{
    SDL_Event event;
    auto quit = false;
    auto delay = static_cast<int>(MILLESECONDS_PER_FRAME);

    while(!quit) {
        try {
            auto startFrame = SDL_GetTicks();
            memset(&event, 0, sizeof(SDL_Event));
            auto screenUpdateDone = false;

            while(_eventManager->WaitEvent(&event, delay) != 0) {
                startFrame = SDL_GetTicks();
                HandleEvent(event, quit);
                if(!quit) {
                    UpdateScreens();
                    screenUpdateDone = true;
                }
            }
            
            if(!screenUpdateDone) {
                startFrame = SDL_GetTicks();
                UpdateScreens();
            } 

            const auto endFrame = SDL_GetTicks();

            /* figure out how much time we have left, and then sleep */
            delay = static_cast<int>(MILLESECONDS_PER_FRAME - (endFrame - startFrame));
            if(delay < 0) {
                delay = 0;
            } else if(delay > MILLESECONDS_PER_FRAME) {
                delay = static_cast<int>(MILLESECONDS_PER_FRAME);
            }

            if(_callbackState != nullptr && _firstrun) {
                _firstrun = false;
                _callbackState(KernelState::Startup);
            }
        } catch(GUIException& exp) {
            LOG(ERROR) << "GUI Error " << exp.what();
            quit = true;
        } catch(SDLException& exp) {
            LOG(ERROR) << "SDL Error " << exp.what();
            quit = true;
        } catch(TTFException& exp) {
            LOG(ERROR) << "TTF Error " << exp.what();
            quit = true;
        }
        /*catch (SqliteException &exp)
        {
            LOG(ERROR) << "Sqlite Error " << exp.what();
            quit = true;
        }*/
        catch(IllegalStateException& exp) {
            LOG(ERROR) << "State Error " << exp.what();
            quit = true;
        } catch(std::exception& exp) {
            LOG(ERROR) << "Error " << exp.what();
            quit = true;
        }
    }

    if(_callbackState != nullptr) {
        _callbackState(KernelState::Shutdown);
    }
}

void MiniKernel::StartCoreServices()
{
#ifdef LIBOSMSCOUT
    _mapManager = new MapManager();
#endif
}

void MiniKernel::Shutdown()
{
    auto screenEntry = _screens.begin();
    while(screenEntry != _screens.end()) {
        screenEntry->second->Shutdown();
        delete screenEntry->second;
        ++screenEntry;
    }
    _screens.clear();

    if(_mapManager != nullptr) {
        _mapManager->DeInit();
        delete _mapManager;
        _mapManager = nullptr;
    }

    if(_audioManager != nullptr) {
        delete _audioManager;
        _audioManager = nullptr;
    }

    if(_eventManager != nullptr) {
        delete _eventManager;
        _eventManager = nullptr;
    }

    if(_base != nullptr) {
        delete _base;
        _base = nullptr;
    }
}

GUIElementManager*
MiniKernel::CreateScreen(const std::string& title, const std::string& videoDriver, const std::string& backgroundImage, bool fullscreen)
{
#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerInitVideo, "InitVideo", VLOG_IS_ON(4));
#endif
    _screenDpi = _base->InitVideo(videoDriver);
    auto screen = new GUIScreen();

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    TIMED_SCOPE_IF(timerCreateScreen, "CreateScreen", VLOG_IS_ON(4));
#endif

    _manager = screen->Create(title, _eventManager, _mapManager, backgroundImage, fullscreen);
    auto id = screen->GetId();

    _screens.insert(std::make_pair(id, screen));

    if(_mapManager != nullptr) {
        _mapManager->SetScreenDpi(_screenDpi);
        _mapManager->SetMarkerImageFile(_kernelConfig.markerImageFile);
        if(!_kernelConfig.mapDataPath.empty()) {
            if(_mapManager->Init(_kernelConfig.mapDataPath, _kernelConfig.mapStyle,
                                 _kernelConfig.mapIconPaths) != 0) {
                LOG(ERROR) << "mapManager Init Failed";
            } else {
                if(_kernelConfig.startMapPosition.GetLat() != 0 || _kernelConfig.startMapPosition.GetLon() != 0) {
                    _mapManager->CenterMap(_kernelConfig.startMapPosition.GetLat(), _kernelConfig.startMapPosition.GetLon(), -1, 0);
                }
            }
        }
    }

    return _manager;
}

void MiniKernel::SetStateCallBack(KernelStateCallbackFunction callback)
{
    _callbackState = callback;
}

void MiniKernel::RegisterApplicationEvent(ApplicationEventCallbackFunction callbackFunction)
{
    _applicationEventCallbackFunction = callbackFunction;
}

void MiniKernel::DrawTextOnBootScreen(const std::string& text)
{
    const auto screenPtr = _screens.begin();
    if(screenPtr != _screens.end()) {
        screenPtr->second->DrawTextOnBootScreen(text);
        screenPtr->second->UpdateAnimationInternal();
        if(screenPtr->second->NeedRedraw()) {
            screenPtr->second->Draw();
        }
    }
}

int MiniKernel::StartAudio(const std::string& drivername)
{
    _base->InitAudio(drivername);

#ifdef ENABLEAUDIOMANAGER
    _audioManager = new MiniAudioManager(_eventManager, _kernelConfig.lastMusikVolume);
    auto result = _audioManager->Init();
    if(result != 0) {
        LOG(ERROR) << "Audio Init Failed";
        // Todo show on screen
        return result;
    }
    LOG(INFO) << "AudioManager Started";
#endif

    return 0;
}

SDLEventManager* MiniKernel::GetEventManager() const
{
    return _eventManager;
}

void MiniKernel::ShowErrorMessage(const std::string& message)
{
    _errorMessage = message;
    _eventManager->PushKernelEvent(KernelEvent::ShowError);
}

int MiniKernel::PlaySound(const std::string& filename) const
{
    if(!utils::FileExists(filename)) {
        throw FileNotFoundException("Audio File Not Exits");
    }

#ifdef ENABLEAUDIOMANAGER
    if(_audioManager == nullptr) {
        throw NullPointerException("No Audio Manager");
    }
    return _audioManager->PlayBackground(filename);
#else
    return 0;
#endif
}

void MiniKernel::SetConfig(KernelConfig config)
{
    _kernelConfig = config;
}