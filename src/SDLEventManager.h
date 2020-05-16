#pragma once

#include <SDL.h>
#include "IBackendConnect.h"
#include "../common/json/json.hpp"

using json = nlohmann::json;

enum class AppEvent;

enum class KernelState : unsigned char {
	Startup,
	Shutdown,
};

enum class KernelEvent : Sint32 {
    Shutdown = 1
};

std::ostream& operator<<(std::ostream& os, const KernelState c);
std::ostream& operator<<(std::ostream& os, const KernelEvent c);

class SDLEventManager
{
	SDL_mutex* eventLock_;
	SDL_cond* eventWait_;
	SDL_TimerID eventTimer_;

	Uint32 kernelEventType_;
	Uint32 applicationEventType_;
	SendToBackendDelegate _callback;
	IBackendConnect* _backend;

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
};


