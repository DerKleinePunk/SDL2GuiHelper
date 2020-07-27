#pragma once
#include "../SDLEventManager.h"
#include "IAudioManager.h"
#include <SDL_mixer.h>

class MiniAudioManager : public IAudioManager
{
  private:
    bool _initDone;
    int _musikVolume;
    int _hardwareChannels;
    int _audioBufferSize;
    Uint16 _hardwareFormat;
    int _hardwareRate;
    const SDLEventManager* _eventManager;
    Mix_Chunk* _chunk[4];

    void BackgroundChannelDone(const int channel);

  public:
    MiniAudioManager(const SDLEventManager* eventManager, const int musikVolume);
    virtual ~MiniAudioManager();

    int Init();

    bool UpdateUi(GUIRenderer* renderer, GUITexture* screen) const;
    int PlayBackground(const std::string& fileName);
    void GetMediaPlayTimes(int64_t* totalTime, int64_t* currentTime) const;
};
