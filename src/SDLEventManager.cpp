/**
* @file  SDLEventManager.cpp
*
* Implementation for Managed SDLEvents Threadsave
*/

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "SDLEventManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../common/easylogging/easylogging++.h"
#include "SDLEventManager.h"
#include "exception/SDLException.h"

std::ostream& operator<<(std::ostream& os, const KernelState c)
{
	switch (c)
	{
		case KernelState::Startup: os << "Startup";    break;
		case KernelState::Shutdown: os << "Shutdown"; break;
		default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const KernelEvent c)
{
	switch (c)
	{
		case KernelEvent::Shutdown: os << "Shutdown"; break;
		default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

SDLEventManager::SDLEventManager():
	eventLock_(nullptr),
	eventWait_(nullptr),
	eventTimer_(0),
	kernelEventType_(static_cast<Uint32>(-1)),
	applicationEventType_(static_cast<Uint32>(-1))
{
	//logger_ = 
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

SDLEventManager::~SDLEventManager()
{
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
	if (eventLock_ == nullptr) {
		throw SDLException("CreateMutex");
	}

	eventWait_ = SDL_CreateCond();
	if (eventWait_ == nullptr) {
		throw SDLException("CreateCond");
	}

	kernelEventType_ = GetNewEventType();
	applicationEventType_ = GetNewEventType();
	return true;
}

bool SDLEventManager::IsKernelEvent(const SDL_Event* event, KernelEvent& type) const
{
	if(event->type == kernelEventType_)
	{
		type = static_cast<KernelEvent>(event->user.code);
		return true;
	}
	return false;
}

bool SDLEventManager::IsApplicationEvent(const SDL_Event* event, AppEvent& appevent, void*& data1, void*& data2) const
{
	if (event->type == applicationEventType_)
	{
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
	if (result == static_cast<Uint32>(-1)) {
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
	if (result < 0) {
        LOG(ERROR) << "PushEvent Failed " << SDL_GetError();
		SDL_UnlockMutex(eventLock_);
		return false;
	}
	else if (result == 0) {
		LOG(WARNING) << "Event filtered";
		SDL_UnlockMutex(eventLock_);
		return false;
	}
	else {
		LOG(DEBUG) << "Event Pushed";
	}
	SDL_UnlockMutex(eventLock_);
	SDL_CondSignal(eventWait_);
	return true;
}

int SDLEventManager::WaitEvent(SDL_Event* event, Uint32 timeout) const {
    auto result = 0;

    SDL_LockMutex(eventLock_);
	result = SDL_PollEvent(event);
	if (result == 0) {
		if (SDL_CondWaitTimeout(eventWait_, eventLock_, timeout) == SDL_MUTEX_TIMEDOUT) {
			result = 0;
		}
		else {
			result = SDL_PollEvent(event);
		}
	}
	SDL_UnlockMutex(eventLock_);
	SDL_CondSignal(eventWait_);

	return result;
}

bool SDLEventManager::PushKernelEvent(KernelEvent event) const
{
	return PushEvent(kernelEventType_, 0, static_cast<Sint32>(event), nullptr, nullptr);
}

bool SDLEventManager::PushApplicationEvent(AppEvent event, void* data1, void* data2) const
{
	return PushEvent(applicationEventType_, 0, static_cast<Sint32>(event), data1, data2);
}

void SDLEventManager::RegisterBackend(SendToBackendDelegate callcack, IBackendConnect* backend) {
	_callback = callcack;
	_backend = backend;
}

bool SDLEventManager::PushBackendMessage(json const& Message) const {
	if(_callback != nullptr) {
		_callback(Message);
		return true;
	}
	return false;
}

bool SDLEventManager::RegisterMeForBackendMessage(MessageFromBackendDelegate callback) {
	if(_backend != nullptr) {
		_backend->RegisterMeForBackendMessage(callback);
		return true;
	}
	return false;
}