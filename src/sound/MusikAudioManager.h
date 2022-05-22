#pragma once
#include "../SDLEventManager.h"
#include "IAudioManager.h"
#include <SDL_mixer.h>
#include "MediaStream.h"

typedef struct MusikStreamState {
	Mix_Fading fading;
	int fade_step;
	int fade_steps;
	bool pause_at_end;
} MusikStreamState;

class MusikAudioManager : public IAudioManager
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
    std::string _musikfile;
	  MusikStreamState _stream_state;
	  MediaStream*  _current_musik_stream;
	  Uint8* _mixDataBuffer;
	  int _mixDataBufferSize;
	  bool _stopMedia;
    SDL_Thread* _musikdecoderthread;
    int _ms_per_step; // Used to calculate fading steps
    
    void FadeOutMusic(int msec, bool pauseAtEnd);
    void FadeInMusic(int msec);
    void BackgroundChannelDone(const int channel);

  public:
    MusikAudioManager(const SDLEventManager* eventManager, const int musikVolume);
    virtual ~MusikAudioManager();

    int DecoderThreadMain();
	  void MixerCallback(Uint8 *stream, int len);

    //The Interface
    int Init();
    bool UpdateUi(GUIRenderer* renderer, GUITexture* screen) const;
    int PlayBackground(const std::string& fileName);
    void GetMediaPlayTimes(int64_t* totalTime, int64_t* currentTime) const;
    int PlayMusik(const std::string& filename);
};
