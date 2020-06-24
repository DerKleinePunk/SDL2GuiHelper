#pragma once

#include <SDL.h>
#include "IBackendConnect.h"
#include "../common/json/json.hpp"
#include <AppEvents.h>

using json = nlohmann::json;

enum class KernelState : unsigned char {
	Startup,
	Shutdown,
};

enum class KernelEvent : Sint32 {
    Shutdown = 1,
	ShowError = 2,	
};

std::ostream& operator<<(std::ostream& os, const KernelState c);
std::ostream& operator<<(std::ostream& os, const KernelEvent c);

class SDLEventManager;

struct userTimer {
	SDL_TimerID id;
	AppEvent event;
	SDLEventManager* Owner;
	bool oneTime;
};

class SDLEventManager
{
	SDL_mutex* eventLock_;
	SDL_cond* eventWait_;

	Uint32 kernelEventType_;
	Uint32 applicationEventType_;
	SendToBackendDelegate _callback;
	IBackendConnect* _backend;

	userTimer _userTimers[10];
public:
	SDLEventManager();
	~SDLEventManager();
	bool Init();
	bool IsKernelEvent(const SDL_Event* event, KernelEvent& type) const;
	bool IsApplicationEvent(const SDL_Event* event, AppEvent& appevent, void*& data1, void*& data2) const;

	static Uint32 GetNewEventType();
	bool PushEvent(Uint32 type, Uint32 windowID, Sint32 code, void* data1, void* data2) const;
	int WaitEvent(SDL_Event* event, Uint32 timeout) const;

	bool PushKernelEvent(KernelEvent event) const;
	bool PushApplicationEvent(AppEvent event, void* data1, void* data2) const;

	void RegisterBackend(SendToBackendDelegate callcack, IBackendConnect* backend);
	bool PushBackendMessage(json const& Message) const;
	bool RegisterMeForBackendMessage(MessageFromBackendDelegate callback);

	SDL_TimerID CreateTimer(AppEvent event, Uint32 delay, bool oneTime);
	void RemoveTimer(SDL_TimerID id);
	void RemoveTimer(SDL_TimerID id, bool fromCallback);
};


