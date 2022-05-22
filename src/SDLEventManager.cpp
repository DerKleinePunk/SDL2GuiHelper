/**
 * @file  SDLEventManager.cpp
 *
 * Implementation for Managed SDLEvents Threadsave
 */

#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "SDLEventManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "SDLEventManager.h"
#include "../common/easylogging/easylogging++.h"
#include "exception/SDLException.h"

std::ostream& operator<<(std::ostream& os, const KernelState c)
{
    switch(c) {
    case KernelState::Startup:
        os << "Startup";
        break;
    case KernelState::Shutdown:
        os << "Shutdown";
        break;
    default:
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const KernelEvent c)
{
    switch(c) {
    case KernelEvent::Shutdown:
        os << "Shutdown";
        break;
    default:
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

Uint32 timerCallbackfunc(Uint32 interval, void *param) 
{
    auto entry = static_cast<userTimer*>(param);
    std::string name("TimerID");
    name += std::to_string((int32_t)entry->event);
    el::Helpers::setThreadName(name);

	entry->Owner->PushApplicationEvent(entry->event, nullptr, nullptr);
    if(entry->oneTime) {
        entry->Owner->RemoveTimer(entry->id, true);
        return 0;
    }
	return interval; // wenn return 0 cancel Timer
}

SDLEventManager::SDLEventManager()
    : eventLock_(nullptr), eventWait_(nullptr), kernelEventType_(static_cast<Uint32>(-1)),
      applicationEventType_(static_cast<Uint32>(-1)),_backend(nullptr)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    for(size_t i = 0; i < 10; i++) {
        _userTimers[i].id = -1;
		_userTimers[i].Owner = nullptr;
    }
}

SDLEventManager::~SDLEventManager()
{
	for(size_t i = 0; i < 10; i++) {
        if(_userTimers[i].id != -1) {
			SDL_RemoveTimer(_userTimers[i].id);
		}
    }

    SDL_DestroyMutex(eventLock_);
    eventLock_ = nullptr;

    SDL_DestroyCond(eventWait_);
    eventWait_ = nullptr;
}

bool SDLEventManager::Init()
{
    if(eventLock_ != nullptr) {
        throw SDLException("do not init twice");
    }

    eventLock_ = SDL_CreateMutex();
    if(eventLock_ == nullptr) {
        throw SDLException("CreateMutex");
    }

    eventWait_ = SDL_CreateCond();
    if(eventWait_ == nullptr) {
        throw SDLException("CreateCond");
    }

    kernelEventType_ = GetNewEventType();
    applicationEventType_ = GetNewEventType();
    return true;
}

bool SDLEventManager::IsKernelEvent(const SDL_Event* event, KernelEvent& type, void*& data1, void*& data2) const
{
    if(event->type == kernelEventType_) {
        type = static_cast<KernelEvent>(event->user.code);
        data1 = event->user.data1;
        data2 = event->user.data2;
        return true;
    }
    return false;
}

bool SDLEventManager::IsApplicationEvent(const SDL_Event* event, AppEvent& appevent, void*& data1, void*& data2) const
{
    if(event->type == applicationEventType_) {
        appevent = static_cast<AppEvent>(event->user.code);
        data1 = event->user.data1;
        data2 = event->user.data2;
        return true;
    }
    return false;
}

Uint32 SDLEventManager::GetNewEventType()
{
    auto result = SDL_RegisterEvents(1);
    if(result == static_cast<Uint32>(-1)) {
        throw SDLException("RegisterEvents");
    }
    return result;
}

bool SDLEventManager::PushEvent(Uint32 type, Uint32 windowID, Sint32 code, void* data1, void* data2) const
{
    if(eventLock_ == nullptr) return false;

    SDL_LockMutex(eventLock_);
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = type;
    event.user.code = code;
    event.user.type = type;
    event.window.windowID = windowID;
    event.user.windowID = windowID;
    event.user.data1 = data1;
    event.user.data2 = data2;
    auto result = SDL_PushEvent(&event);
    if(result < 0) {
        LOG(ERROR) << "PushEvent Failed " << SDL_GetError();
        SDL_UnlockMutex(eventLock_);
        return false;
    } else if(result == 0) {
        LOG(WARNING) << "Event filtered";
        SDL_UnlockMutex(eventLock_);
        return false;
    } else {
        LOG(DEBUG) << "Event Pushed " << std::to_string(type);
    }
    SDL_UnlockMutex(eventLock_);
    SDL_CondSignal(eventWait_);
    return true;
}

/**
 *  \brief Waits timeout max for new event
 *
 *  \return 1, or 0 if there was an error while waiting for events.
 *
 *  \param event not null
 * 
 *  \param timeout in ms
 */
int SDLEventManager::WaitEvent(SDL_Event* event, Uint32 timeout) const
{
    auto result = 0;

    SDL_LockMutex(eventLock_);
    result = SDL_PollEvent(event);
    if(result == 0) {
        if(SDL_CondWaitTimeout(eventWait_, eventLock_, timeout) == SDL_MUTEX_TIMEDOUT) {
            result = SDL_PollEvent(event);
            VLOG(7) << std::to_string(timeout) << " ms we have wait";
        } else {
            result = SDL_PollEvent(event);
            VLOG(7) << "wakeup from mutex";
        }
    }
    SDL_UnlockMutex(eventLock_);
    SDL_CondSignal(eventWait_);

    return result;
}

bool SDLEventManager::PushKernelEvent(KernelEvent event, void* data1, void* data2) const
{
    return PushEvent(kernelEventType_, 0, static_cast<Sint32>(event), data1, data2);
}

bool SDLEventManager::PushApplicationEvent(AppEvent event, void* data1, void* data2) const
{
    return PushEvent(applicationEventType_, 0, static_cast<Sint32>(event), data1, data2);
}

void SDLEventManager::RegisterBackend(SendToBackendDelegate callcack, IBackendConnect* backend)
{
    _callback = callcack;
    _backend = backend;
}

bool SDLEventManager::PushBackendMessage(json const& Message) const
{
    if(_callback != nullptr) {
        _callback(Message);
        return true;
    }
    return false;
}

bool SDLEventManager::RegisterMeForBackendMessage(MessageFromBackendDelegate callback)
{
    if(_backend != nullptr) {
        _backend->RegisterMeForBackendMessage(callback);
        return true;
    }
    return false;
}

SDL_TimerID SDLEventManager::CreateTimer(AppEvent event, Uint32 delay, bool oneTime)
{
	auto freeTimerFound = false;
	size_t i = 0;
	for (i = 0; i < 10; i++)
	{
		if(_userTimers[i].id == -1) {
			freeTimerFound = true;
			break;
		}
	}

	if(!freeTimerFound) return -1;

	//Uint32 delayInt = (33.0 * delay) / delay;  /* To round it down to the nearest X ms */
	_userTimers[i].event = event;
	_userTimers[i].Owner = this;
	_userTimers[i].id = SDL_AddTimer(delay, timerCallbackfunc, &_userTimers[i]);
    _userTimers[i].oneTime = oneTime;

	return _userTimers[i].id;
}

void SDLEventManager::RemoveTimer(SDL_TimerID id)
{
	RemoveTimer(id, false);
}

void SDLEventManager::RemoveTimer(SDL_TimerID id, bool fromCallback)
{
    for(size_t i = 0; i < 10; i++) {
        if(_userTimers[i].id == id) {
            if(!fromCallback) {
			    SDL_RemoveTimer(_userTimers[i].id);
            }
			_userTimers[i].id = -1;
            _userTimers[i].Owner = nullptr;
			return;
		}
    }
}