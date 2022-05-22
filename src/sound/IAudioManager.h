#pragma once
#include <string>
#include "../gui/GUIRenderer.h"
#include "../gui/GUITexture.h"

#define SDL_AUDIO_BUFFER_SIZE 1024
#define FADING_TIME_MS 800
#define HW_SAMPE_RATE 44100

class IAudioManager
{
private:
    
public:
    virtual ~IAudioManager(){};

    virtual int Init() = 0;
    virtual bool UpdateUi(GUIRenderer* renderer, GUITexture* screen) const = 0;
    virtual int PlayBackground(const std::string& fileName) = 0;
    virtual void GetMediaPlayTimes(int64_t* totalTime, int64_t* currentTime) const = 0;

    virtual int PlayMusik(const std::string& filename) = 0;
};


