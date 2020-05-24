#pragma once
#include "../common/easylogging/easylogging++.h"
#include <SDL.h>

class SDLBase
{
    bool initDone_;
	bool initVideoDone_;
	bool initAudioDone_;
    el::Logger* logger_;
	el::Logger* sdlLogger_;

	static int InitSubsystem(uint32_t flags);
	static void LogSystemsRunning();
	void SdlLogEntry(void *userdata, int category, SDL_LogPriority priority, const char *message);
public:
	SDLBase();
	virtual ~SDLBase();

	void Init();
	float InitVideo(const std::string& videoDriver);
	void InitAudio(const std::string& drivername);
};

