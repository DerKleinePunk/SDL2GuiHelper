#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MiniAudioManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MiniAudioManager.h"
#include <functional>
#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/commonutils.h"
#include "../exception/SDLException.h"

void MiniAudioManager::BackgroundChannelDone(const int channel)
{
    LOG(DEBUG) << "BackgroundChannelDone " << channel;
    Mix_FreeChunk(_chunk[channel]);
    /*if(current_musik_stream_!= nullptr && stream_state_.pause_at_end) {
       FadeInMusic(FADING_TIME_MS);*/
}

MiniAudioManager::MiniAudioManager(const SDLEventManager* eventManager, const int musikVolume):
    _initDone(false),
    _musikVolume(musikVolume),
    _hardwareChannels(2),
    _audioBufferSize(0),
    _hardwareFormat(AUDIO_S16SYS),
    _hardwareRate(HW_SAMPE_RATE),
    _eventManager(eventManager)
    
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

MiniAudioManager::~MiniAudioManager()
{
}

int MiniAudioManager::Init()
{
    if(_initDone) return 0;

    LOG(DEBUG) << "Init Audio Manager";
    SDL_version compiled;

    MIX_VERSION(&compiled);
    const auto linked = *Mix_Linked_Version();

    LOG(INFO) << "SDL_mixer Version compiled " << static_cast<int>(compiled.major) << "."
              << static_cast<int>(compiled.minor) << "." << static_cast<int>(compiled.patch);
    LOG(INFO) << "SDL_mixer Version linked " << static_cast<int>(linked.major) << "."
              << static_cast<int>(linked.minor) << "." << static_cast<int>(linked.patch);

    _audioBufferSize = SDL_AUDIO_BUFFER_SIZE;

    if(Mix_OpenAudioDevice(_hardwareRate, _hardwareFormat, _hardwareChannels, _audioBufferSize, nullptr,
                           SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE) < 0) {
        throw SDLException("Mix_OpenAudio");
    }

    if(Mix_QuerySpec(&_hardwareRate, &_hardwareFormat, &_hardwareChannels) != 1) {
        throw SDLException("Mix_QuerySpec");
    }
    const std::string soundText =
    (_hardwareChannels > 2) ? "surround" : (_hardwareChannels > 1) ? "stereo" : "mono";
    const std::string formatText = (_hardwareFormat & 0x1000) ? " BE" : " LE";
    const std::string formatText2 = (_hardwareFormat & 0x8000) ? " signed" : " unsigned";

    LOG(INFO) << "Opened Audio with " << _hardwareRate << "Hz " << (_hardwareFormat & 0xFF) << "bit "
              << soundText << formatText << formatText2 << " Buffersize " << _audioBufferSize;

    const char* driver_name = SDL_GetCurrentAudioDriver();
    if(driver_name) {
        LOG(INFO) << "Audio subsystem initialized; driver = " << driver_name;
    } else {
        LOG(ERROR) << "Audio subsystem not initialized.";
    }

    const auto n = Mix_GetNumMusicDecoders();
    LOG(INFO) << "There are " << n << " available music decoders (SDL2_mixer):";
    for(auto i = 0; i < n; ++i) {
        LOG(INFO) << Mix_GetMusicDecoder(i);
    }

    Mix_VolumeMusic(_musikVolume);

    int channelCount = Mix_AllocateChannels(4);
    if(channelCount != 4) {
        LOG(WARNING) << "we get not all Mixer Channles";
    }

    utils::Callback<void(int)>::func =
    std::bind(&MiniAudioManager::BackgroundChannelDone, this, std::placeholders::_1);
    void (*ChannelDone)(int) = static_cast<decltype(ChannelDone)>(utils::Callback<void(int)>::callback);
    Mix_ChannelFinished(ChannelDone);

    _initDone = true;

    return 0;
}

bool MiniAudioManager::UpdateUi(GUIRenderer* renderer, GUITexture* screen) const
{
    return false;
}

int MiniAudioManager::PlayBackground(const std::string& fileName)
{
    auto current_chunk = Mix_LoadWAV(fileName.c_str());
	if (current_chunk == nullptr) {
		LOG(ERROR) << "Couldn't load file: " << fileName << " " << Mix_GetError();
		return -1;
	} else {
        VLOG(1) << "Try Playing " << fileName;
    }

    auto result = Mix_PlayChannel(-1, current_chunk, 0);
	if (result == -1) {
		// may be critical error, or maybe just no channels were free.
		// you could allocated another channel in that case...
		LOG(ERROR) << "Mix_FadeInChannel: " << Mix_GetError();
		return -1;
	}

	_chunk[result] = current_chunk;

	return 0;
}

void MiniAudioManager::GetMediaPlayTimes(int64_t* totalTime, int64_t* currentTime) const
{
    *totalTime = 0;
    *currentTime = 0;
}

int MiniAudioManager::PlayMusik(const std::string& filename) {
    LOG(WARNING) << "This PLayer can play Musik";
    return -1;
}