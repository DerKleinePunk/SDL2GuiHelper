/**
 * @file  MusikAudioManager.cpp
 *
 * Implementation for Manager MusikAudio Subsystem
 *
 * @date  2015-04-13
 *
 * https://sourceforge.net/p/karlyriceditor/code/HEAD/tree/src/audioplayerprivate.cpp
 * http://stackoverflow.com/questions/21342194/sdl2-ffmpeg2-intermittent-clicks-instead-of-audio
 * https://transcoding.wordpress.com/2011/11/16/careful-with-audio-resampling-using-ffmpeg/
 * http://dranger.com/ffmpeg/
 * http://ffmpeg.zeranoe.com/builds/ geht inculde an so on
 * https://github.com/podshumok/libav-stream-plusplus/blob/master/stream_transcoder.cpp
 * lexical_cast -> boost libary
 */

#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MusikAudioManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MusikAudioManager.h"

#include <functional>

#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/commonutils.h"
#include "../exception/SDLException.h"

#define FADING_TIME_MS 800

static void avlog_cb(void* ptr, int level, const char* fmt, va_list arg_list)
{
    static std::string lastout;
    std::string prefix;

    // if(lastout == "") {
    auto avc = ptr ? *reinterpret_cast<AVClass**>(ptr) : nullptr;
    if(avc) {
        /*if (avc->parent_log_context_offset)
        {
            AVClass** parent = *reinterpret_cast<AVClass ***> (ptr) +
                avc->parent_log_context_offset;
            if (parent && *parent)
            {
                prefix += '[';
                prefix += (*parent)->class_name;
                prefix += ' ';
                prefix += (*parent)->item_name(parent);
                prefix += " @ ";
                //prefix += lexical_cast<std::string>(parent);
                prefix += "] ";
            }
        }*/
        prefix += '[';
        prefix += avc->class_name;
        prefix += ' ';
        prefix += avc->item_name(ptr);
        // prefix += " @ ";
        // prefix += lexical_cast<std::string>(ptr);
        prefix += "] ";
    }
    //}

    lastout += prefix + utils::stringf(fmt, arg_list).get();

    const auto found = lastout.find("\n") != std::string::npos;
    if(!found) return;

    std::replace(lastout.begin(), lastout.end(), '\n', '*');
    std::replace(lastout.begin(), lastout.end(), '\r', '#');
    lastout = "avlog " + lastout;

    // Todo splitt/remove newline in log entry
    switch(level) {
    case AV_LOG_PANIC:
    case AV_LOG_FATAL:
        LOG(FATAL) << lastout;
        break;
    case AV_LOG_ERROR:
        LOG(ERROR) << lastout;
        break;
    case AV_LOG_WARNING:
        LOG(WARNING) << lastout;
        break;
    case AV_LOG_INFO:
        LOG(INFO) << lastout;
        break;
    case AV_LOG_VERBOSE:
        VLOG(3) << lastout;
        break;
    case AV_LOG_DEBUG:
        LOG(DEBUG) << lastout;
        break;
    default:
        LOG(INFO) << lastout;
    }
    lastout = "";
}

static void MusikMixerCallback(void* udata, Uint8* stream, int len)
{
    auto mixer = static_cast<MusikAudioManager*>(udata);
    mixer->MixerCallback(stream, len);
}

static int decode_thread_static(void* arg)
{
    auto mixer = static_cast<MusikAudioManager*>(arg);
    return mixer->DecoderThreadMain();
}

int MusikAudioManager::DecoderThreadMain()
{
    auto state = StreamStates::error;
    LOG(DEBUG) << "decoder thread started";

    _current_musik_stream = new MediaStream();
    auto format = AV_SAMPLE_FMT_S32;
    if((_hardwareFormat & 0xFF) == 16) {
        format = AV_SAMPLE_FMT_S16;
    }
    const auto result = _current_musik_stream->Init(_musikfile, format, _hardwareChannels, _audioBufferSize, _hardwareRate);
    if(result < 0) {
        delete _current_musik_stream;
        _current_musik_stream = nullptr;

        LOG(ERROR) << "Error Loading Musik File";
        _stopMedia = true;
        _eventManager->PushApplicationEvent(AppEvent::MusikStreamError, nullptr, nullptr);
        LOG(DEBUG) << "decoder thread stopped";
        return result;
    }

    Mix_HookMusic(&MusikMixerCallback, this);

    const auto streamInfo = new AudioStreamInfo();
    _current_musik_stream->GetMetaData(streamInfo);

    _eventManager->PushApplicationEvent(AppEvent::MusikStreamPlay, streamInfo, nullptr);

    while(!_stopMedia) {
        // seek stuff goes here
        // if (is->seek_req) {
        // current_musik_stream_->Seek(seek_pos);
        // is->seek_req = 0;
        //}

        state = _current_musik_stream->FillStreams();
        if(state == StreamStates::bufferfull) {
            // SDL_Delay(10); We wait in FillStreams now
        }
        if(state == StreamStates::allloaded) {
            LOG(DEBUG) << "allloaded";
            // Todo find an way without delay
            SDL_Delay(10);
        }
        if(state == StreamStates::finish) {
            break;
        }
        if(state == StreamStates::error) {
            LOG(ERROR) << "Error in FillStreams";
            break;
        }
    }

    LOG(DEBUG) << "decoder thread exit loop";

    Mix_HookMusic(nullptr, nullptr);

    delete _current_musik_stream;
    _current_musik_stream = nullptr;

    if(_mixDataBufferSize > 0) {
        delete[] _mixDataBuffer;
        _mixDataBufferSize = 0;
        _mixDataBuffer = nullptr;
    }

    _stopMedia = true;
    if(state == StreamStates::error) {
        if(!_eventManager->PushApplicationEvent(AppEvent::MusikStreamError, nullptr, nullptr)) {
            LOG(ERROR) << "PushEvent failed";
        }
    } else {
        if(!_eventManager->PushApplicationEvent(AppEvent::MusikStreamStopp, nullptr, nullptr)) {
            LOG(ERROR) << "PushEvent failed";
        }
    }

    LOG(DEBUG) << "decoder thread stopped";

    return 0;
}

void MusikAudioManager::MixerCallback(Uint8* stream, int len)
{
    // Todo is wenn Fadeout the Fade flg ist set back but the Stream sends new packet we have an Blob Sound
    // SDL_memset(stream, 0, len) SDL_Mixer do this
    if(_current_musik_stream && _current_musik_stream->IsReady()) {
        if(_mixDataBufferSize < len) {
            if(_mixDataBuffer != nullptr) delete[] _mixDataBuffer;
            _mixDataBuffer = new Uint8[len];
            _mixDataBufferSize = len;
        }
        auto musikCurrentVolume = _musikVolume;

        _current_musik_stream->GetAudioData(_mixDataBuffer, len);

        if(_stream_state.fading != MIX_NO_FADING) {
            const auto fadeStep = _stream_state.fade_step;
            const auto fadeSteps = _stream_state.fade_steps;

            VLOG(4) << "Fading " << _stream_state.fading << " " << fadeStep << " " << fadeSteps;

            if(_stream_state.fading == MIX_FADING_OUT) {
                musikCurrentVolume = (_musikVolume * (fadeSteps - fadeStep)) / fadeSteps;
            } else { /* Fading in */
                musikCurrentVolume = (_musikVolume * fadeStep) / fadeSteps;
            }

            VLOG(4) << "Fading " << _stream_state.fading << " " << fadeStep << " " << fadeSteps << " " << musikCurrentVolume;

            if(_stream_state.fade_step++ < _stream_state.fade_steps) {
            } else {
                if(_stream_state.fading == MIX_FADING_OUT) {
                    if(_stream_state.pause_at_end) {
                        _current_musik_stream->Pause();
                    } else {
                        _stopMedia = true;
                    }
                }
                _stream_state.fading = MIX_NO_FADING;
            }
        }

        SDL_MixAudioFormat(stream, _mixDataBuffer, _hardwareFormat, len, musikCurrentVolume);
        const auto lastError = SDL_GetError();
        if(strlen(lastError) > 0) {
            LOG(ERROR) << "SDL_MixAudioFormat " << lastError;
            SDL_ClearError();
        }
    }
}

void MusikAudioManager::FadeOutMusic(const int msec, const bool pauseAtEnd)
{
    const auto fadeSteps = (msec + _ms_per_step - 1) / _ms_per_step;
    if(_stream_state.fading == MIX_NO_FADING) {
        _stream_state.fade_step = 0;
    } else {
        int step;
        const auto oldFadeSteps = _stream_state.fade_steps;
        if(_stream_state.fading == MIX_FADING_OUT) {
            step = _stream_state.fade_step;
        } else {
            step = oldFadeSteps - _stream_state.fade_step + 1;
        }
        _stream_state.fade_step = step * fadeSteps / oldFadeSteps;
    }

    _stream_state.fading = MIX_FADING_OUT;
    _stream_state.fade_steps = fadeSteps;
    _stream_state.pause_at_end = pauseAtEnd;
}

void MusikAudioManager::FadeInMusic(int msec)
{
    const auto fading = _stream_state.fading;
    const auto fadeSteps = (msec + _ms_per_step - 1) / _ms_per_step;
    if(_stream_state.fading == MIX_NO_FADING) {
        _stream_state.fade_step = 0;
    } else {
        int step;
        const auto oldFadeSteps = _stream_state.fade_steps;
        if(_stream_state.fading == MIX_FADING_OUT) {
            step = (fadeSteps - _stream_state.fade_step) + 2;
        } else {
            step = oldFadeSteps - _stream_state.fade_step + 1;
        }
        _stream_state.fade_step = step * fadeSteps / oldFadeSteps;
    }

    _stream_state.fading = MIX_FADING_IN;
    _stream_state.fade_steps = fadeSteps;
    if(fading == MIX_NO_FADING) {
        _current_musik_stream->Resume();
    }
}

void MusikAudioManager::BackgroundChannelDone(const int channel)
{
    LOG(DEBUG) << "BackgroundChannelDone " << channel;
    Mix_FreeChunk(_chunk[channel]);
    if(_current_musik_stream != nullptr && _stream_state.pause_at_end) {
        FadeInMusic(FADING_TIME_MS);
    }
}

MusikAudioManager::MusikAudioManager(const SDLEventManager* eventManager, const int musikVolume)
    : _initDone(false), _musikVolume(musikVolume), _hardwareChannels(2), _audioBufferSize(0), _hardwareFormat(AUDIO_S16SYS),
      _hardwareRate(HW_SAMPE_RATE), _eventManager(eventManager), _mixDataBuffer(nullptr), _mixDataBufferSize(0)

{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _stream_state.fading = MIX_NO_FADING;
    _stopMedia = true;
    _musikdecoderthread = nullptr;
}

MusikAudioManager::~MusikAudioManager()
{
}

int MusikAudioManager::Init()
{
    if(_initDone) return 0;

    LOG(DEBUG) << "Init Audio Manager";

    av_log_set_callback(avlog_cb);

#ifdef DEBUG
    av_log_set_level(AV_LOG_DEBUG);
    // Todo get it from Config
#endif
#if(LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100))
    avcodec_register_all();
    av_register_all();
#endif

    avformat_network_init();

    SDL_version compiled;

    MIX_VERSION(&compiled);
    const auto linked = *Mix_Linked_Version();

    LOG(INFO) << "SDL_mixer Version compiled " << static_cast<int>(compiled.major) << "."
              << static_cast<int>(compiled.minor) << "." << static_cast<int>(compiled.patch);
    LOG(INFO) << "SDL_mixer Version linked " << static_cast<int>(linked.major) << "." << static_cast<int>(linked.minor)
              << "." << static_cast<int>(linked.patch);

    _audioBufferSize = SDL_AUDIO_BUFFER_SIZE;

    if(Mix_OpenAudioDevice(_hardwareRate, _hardwareFormat, _hardwareChannels, _audioBufferSize, nullptr,
                           SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE) < 0) {
        throw SDLException("Mix_OpenAudio");
    }

    if(Mix_QuerySpec(&_hardwareRate, &_hardwareFormat, &_hardwareChannels) != 1) {
        throw SDLException("Mix_QuerySpec");
    }
    const std::string soundText = (_hardwareChannels > 2) ? "surround" : (_hardwareChannels > 1) ? "stereo" : "mono";
    const std::string formatText = (_hardwareFormat & 0x1000) ? " BE" : " LE";
    const std::string formatText2 = (_hardwareFormat & 0x8000) ? " signed" : " unsigned";

    LOG(INFO) << "Opened Audio with " << _hardwareRate << "Hz " << (_hardwareFormat & 0xFF) << "bit " << soundText
              << formatText << formatText2 << " Buffersize " << _audioBufferSize;

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

    utils::Callback<void(int)>::func = std::bind(&MusikAudioManager::BackgroundChannelDone, this, std::placeholders::_1);
    void (*ChannelDone)(int) = static_cast<decltype(ChannelDone)>(utils::Callback<void(int)>::callback);
    Mix_ChannelFinished(ChannelDone);

    _ms_per_step = static_cast<int>(static_cast<float>(_audioBufferSize) * 1000.0 / _hardwareRate);

    _initDone = true;

    return 0;
}

bool MusikAudioManager::UpdateUi(GUIRenderer* renderer, GUITexture* screen) const
{
    if(_current_musik_stream && _current_musik_stream->IsReady()) {
        return _current_musik_stream->UpdateUi(renderer, screen);
    }
    return false;
}

int MusikAudioManager::PlayBackground(const std::string& fileName)
{
    auto current_chunk = Mix_LoadWAV(fileName.c_str());
    if(current_chunk == nullptr) {
        LOG(ERROR) << "Couldn't load file: " << Mix_GetError();
        return -1;
    }

    auto result = Mix_PlayChannel(-1, current_chunk, 0);
    if(!_stopMedia) {
        FadeOutMusic(FADING_TIME_MS, true);
        result = Mix_PlayChannel(-1, current_chunk, 0);
    } else {
        result = Mix_FadeInChannel(-1, current_chunk, 0, FADING_TIME_MS);
    }
    // -1 means search free channel we have 4 see init
    // if 'loops' is greater than zero, loop the sound that many times.
    // If 'loops' is -1, loop inifinitely (~65000 times).
    if(result == -1) {
        // may be critical error, or maybe just no channels were free.
        // you could allocated another channel in that case...
        LOG(ERROR) << "Mix_FadeInChannel: " << Mix_GetError();
        return -1;
    }

    _chunk[result] = current_chunk;

    return 0;
}

int MusikAudioManager::PlayMusik(const std::string& filename)
{
    if(filename.empty()) {
        LOG(ERROR) << "PlayMusik FileName is Empty";
        return -10;
    }

    LOG(INFO) << "PlayMusik " << filename;

    if(!_initDone) return -9;

    if(!_stopMedia) {
        LOG(DEBUG) << "we are bussy we can only play one musik file at one time";
        return -1;
    }

    if(_musikdecoderthread != nullptr) {
        int ret;
        LOG(DEBUG) << "Wait decoder Thread";
        SDL_WaitThread(_musikdecoderthread, &ret);
    }

    _musikfile = filename;
    _stopMedia = false;
    _stream_state.fade_step = 0;
    _stream_state.fade_steps = 0;
    _stream_state.fading = MIX_NO_FADING;
    _musikdecoderthread = SDL_CreateThread(decode_thread_static, "mixerdecoder", this);

    LOG(DEBUG) << "Decoder Thread Started " << filename;
    return 0;
}

void MusikAudioManager::GetMediaPlayTimes(int64_t* totalTime, int64_t* currentTime) const
{
    if(_current_musik_stream && _current_musik_stream->IsReady()) {
        _current_musik_stream->GetPlayTimes(totalTime, currentTime);
    }
}