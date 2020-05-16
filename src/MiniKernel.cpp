#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "MiniKernel"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MiniKernel.h"
#include "../common/easylogging/easylogging++.h"
#include "gui/GUIScreen.h"
#include "exception/GUIException.h"
#include "exception/SDLException.h"
#include "exception/TTFException.h"
#include "../common/exception/IllegalStateException.h"

//#define MILLESECONDS_PER_FRAME 1000.0/120.0       /* about 120 frames per second */
#define MILLESECONDS_PER_FRAME 1000.0/60.0       /* about 60 frames per second */

void MiniKernel::HandleEvent(const SDL_Event& event,bool& exitLoop) {
    GUIScreen* screen = nullptr;

    auto winId = event.window.windowID;
    if (winId == 0) {
        winId = event.user.windowID;
    }
    
    if (event.type == SDL_FINGERMOTION ||
        event.type == SDL_FINGERDOWN ||
        event.type == SDL_FINGERUP) {
        //Todo how to map touchId to Window
        LOG(DEBUG) << "Touch Id " << event.tfinger.touchId;
        winId = 1;
	}

	if (_screens.find(winId) != _screens.end()) {
		screen = _screens[winId];
	}
	
	if(screen) {
		screen->HandleEvent(&event);
	}
	
	KernelEvent type;
	if (_eventManager->IsKernelEvent(&event, type)) {
		switch (type)
		{
			case KernelEvent::Shutdown:
            {
                LOG(INFO) << "Kernel Event Shutdown";
                exitLoop = true;
                break;
            }
			default:
            {
                LOG(WARNING) << "Not Implemented Kernel Event " << type;
            }
		}
	}

	AppEvent code;
	void* data1;
	void* data2;
	if (_eventManager->IsApplicationEvent(&event, code, data1, data2)) {
        //TODO better move to Kernel event ?
        //TODO better move Interface ?
        /*if(code == AppEvent::NewGeopos && mapManager_ != nullptr) {
            KernelGPSMessage* message = (KernelGPSMessage*)data1;
            configManager_->UpdateLastPosition(message->coord);
            mapManager_->CenterMap(message->coord, message->compass, message->speed);
        } else if(code == AppEvent::LongClick || code == AppEvent::Click) {
            PlaySound("Click.wav");
        } else {
            
        }*/
        if (_applicationEventCallbackFunction != nullptr) {
            _applicationEventCallbackFunction(code, data1, data2);
        }
	}

	switch (event.type)
	{
		case SDL_QUIT:
			exitLoop = true;
			break;
        case SDL_KEYDOWN: {
                const Uint8 *state = SDL_GetKeyboardState(NULL);
                switch (event.key.keysym.sym) {
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
			//LOG(DEBUG) << event.type << " unhandelt event type";
			break;
	}
}

MiniKernel::MiniKernel(){
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _base = nullptr;
    _eventManager = nullptr;
    _firstrun = true;
    _callbackState = nullptr;
    _screenDpi = 0.0;
    _applicationEventCallbackFunction = nullptr;
}

MiniKernel::~MiniKernel()
{
}

bool MiniKernel::StartUp(int argc, char* argv[]) {
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

void MiniKernel::Run() {
    SDL_Event event;
    auto quit = false;
    auto delay = static_cast<int>(MILLESECONDS_PER_FRAME);

    while (!quit) {
        try {
            const auto startFrame = SDL_GetTicks();
            while (_eventManager->WaitEvent(&event, delay) != 0) {
                HandleEvent(event, quit);
                auto winId = event.window.windowID;
                if (winId == 0) {
                    winId = event.user.windowID;
                }

                if (winId == 0) {
                    //Update all Windows
                    auto screenPtr = _screens.begin();
                    while (screenPtr != _screens.end()) {
                        screenPtr->second->UpdateAnimationInternal();
                        if (screenPtr->second->NeedRedraw())
                        {
                            screenPtr->second->Draw();
                        }
                        ++screenPtr;
                    }
                } else {
                    const auto screenPtr = _screens.find(winId);
                    if (screenPtr != _screens.end()) {
                        screenPtr->second->UpdateAnimationInternal();
                        if (screenPtr->second->NeedRedraw()) {
                                screenPtr->second->Draw();
                        }
                    }
                }
            }

            auto screenPtr = _screens.begin();
            while (screenPtr != _screens.end()) {
                screenPtr->second->UpdateAnimationInternal();
                if (screenPtr->second->NeedRedraw()) {
                    screenPtr->second->Draw();
                }
                ++screenPtr;
            }

            const auto endFrame = SDL_GetTicks();

            /* figure out how much time we have left, and then sleep */
            delay = static_cast<int>(MILLESECONDS_PER_FRAME - (endFrame - startFrame));
            if (delay < 0) {
                delay = 0;
            }
            else if (delay > MILLESECONDS_PER_FRAME) {
                delay = static_cast<int>(MILLESECONDS_PER_FRAME);
            }

            if (_callbackState != nullptr && _firstrun) {
                _firstrun = false;
                _callbackState(KernelState::Startup);
            }
        }
        catch (GUIException &exp)
        {
            LOG(ERROR) << "GUI Error " << exp.what();
            quit = true;
        }
        catch (SDLException &exp)
        {
            LOG(ERROR) << "SDL Error " << exp.what();
            quit = true;
        }
        catch (TTFException &exp)
        {
            LOG(ERROR) << "TTF Error " << exp.what();
            quit = true;
        }
        /*catch (SqliteException &exp)
        {
            LOG(ERROR) << "Sqlite Error " << exp.what();
            quit = true;
        }*/
        catch (IllegalStateException &exp)
        {
            LOG(ERROR) << "State Error " << exp.what();
            quit = true;
        }
        catch (std::exception &exp)
        {
            LOG(ERROR) << "Error " << exp.what();
            quit = true;
        }
    }

    if (_callbackState != nullptr) {
        _callbackState(KernelState::Shutdown);
    }
}

void MiniKernel::Shutdown() {

}

GUIElementManager* MiniKernel::CreateScreen(const std::string& title, const std::string& videoDriver) {
    #ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerInitVideo, "InitVideo", VLOG_IS_ON(4));
#endif
    _screenDpi = _base->InitVideo(videoDriver);
	auto screen = new GUIScreen();

#ifdef ELPP_FEATURE_PERFORMANCE_TRACKING
    	TIMED_SCOPE_IF(timerCreateScreen, "CreateScreen", VLOG_IS_ON(4));
#endif

    IMapManager* mapManager = nullptr;

	const auto manager = screen->Create(title, _eventManager, mapManager, "");
	auto id = screen->GetId();

	_screens.insert(std::make_pair(id, screen));
    
    //if(mapManager_ != nullptr) mapManager_->SetScreenDpi(screenDpi_);

	return manager;
}

void MiniKernel::SetStateCallBack(KernelStateCallbackFunction callback) {
    _callbackState = callback;
}

void MiniKernel::RegisterApplicationEvent(ApplicationEventCallbackFunction callbackFunction) {
	_applicationEventCallbackFunction = callbackFunction;
}