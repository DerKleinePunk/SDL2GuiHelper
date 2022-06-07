#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "SDLBase"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "SDLBase.h"
#include <SDL.h>
#include <SDL_image.h>
#include "../common/utils/SDL2Helper.h"
#include "exception/SDLException.h"

namespace utils
{
extern LOGCALLBACK logCallback;
}

SDLBase::SDLBase() : initDone_(false), initVideoDone_(false), initAudioDone_(false), _touches(0)
{
    logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    sdlLogger_ = el::Loggers::getLogger("SDL");
}

SDLBase::~SDLBase()
{
    if(initAudioDone_) SDL_AudioQuit();
    if(initVideoDone_) {
        IMG_Quit();
        SDL_VideoQuit();
    }
    SDL_Quit();
}

void SDLBase::Init()
{
    if(initDone_) return;

    utils::logCallback = std::bind(&SDLBase::SdlLogEntry, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    SDL_LogSetOutputFunction(utils::LogOutputFunction, this);

    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    LOG(INFO) << "SDL Version compiled " << static_cast<int>(compiled.major) << "."
              << static_cast<int>(compiled.minor) << "." << static_cast<int>(compiled.patch);
    LOG(INFO) << "SDL Version linked " << static_cast<int>(linked.major) << "."
              << static_cast<int>(linked.minor) << "." << static_cast<int>(linked.patch);

    // Todo get it from Config
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        throw SDLException("SDL_Init");
    }

    SDL_StopTextInput();
    LogSystemsRunning();

    initDone_ = true;
}

/**
 * @brief Starting SDL2 Video Subsystem
 * @param[in] videoDriver the VideoDriver or emtpy than we search for driver
 * @return the DPI of Screen or default 96 DPI
 */
float SDLBase::InitVideo(const std::string& videoDriver)
{
    // https://stackoverflow.com/questions/57672568/sdl2-on-raspberry-pi-without-x
    if(VLOG_IS_ON(1)) {
        VLOG(1) << "Testing video drivers...";
        std::vector<bool> drivers(SDL_GetNumVideoDrivers());
        for(std::size_t i = 0; i < drivers.size(); ++i) {
            drivers[i] = (0 == SDL_VideoInit(SDL_GetVideoDriver(i)));
            SDL_VideoQuit();
        }

        std::string logLine = "SDL_VIDEODRIVER available:";
        for(std::size_t i = 0; i < drivers.size(); ++i) {
            logLine += " " + std::string(SDL_GetVideoDriver(i));
        }
        VLOG(1) << logLine;

        logLine = "SDL_VIDEODRIVER usable:";
        for(std::size_t i = 0; i < drivers.size(); ++i) {
            if(!drivers[i]) continue;
            logLine += " " + std::string(SDL_GetVideoDriver(i));
        }
        VLOG(1) << logLine;
    }

    if(InitSubsystem(SDL_INIT_VIDEO) < 0) {
        initVideoDone_ = false;
        throw SDLException("InitVideo");
    }

    if(!videoDriver.empty()) {
        if(SDL_VideoInit(videoDriver.c_str()) < 0) {
            initVideoDone_ = false;
            throw SDLException("InitVideo");
        }
    }

    const auto driver = SDL_GetCurrentVideoDriver();
    LOG(INFO) << "Using " << driver << " Video driver";

    initVideoDone_ = true;

    _touches = SDL_GetNumTouchDevices();
    LOG(INFO) << "TouchDevices " << std::to_string(_touches);

    if(strcmp(driver, "RPI") == 0 || strcmp(driver, "KMSDRM") == 0 ) {
        // todo Change thinks we are on Raspberry without X driver var MemoryLeak ?
        LOG(INFO) << "Raspberry without X";

#ifndef DEBUG 
        if(_touches > 0) {
            LOG(INFO) << "Try Disable the Mouse Cursor";
            SDL_ShowCursor(SDL_DISABLE);
        }
#endif

    }
    // Todo https://wiki.libsdl.org/SDL_GetWindowWMInfo

    auto countOfDrivers = SDL_GetNumRenderDrivers();
    for(auto i = 0; i < countOfDrivers; ++i) {
        SDL_RendererInfo info;
        if(SDL_GetRenderDriverInfo(i, &info) != 0) {
            LOG(ERROR) << "SDL_GetRenderDriverInfo failed:" << SDL_GetError();
            continue;
        }
        LOG(INFO) << "Render Driver " << info.name;
    }

    auto countOfDisplays = SDL_GetNumVideoDisplays();
    for(auto i = 0; i < countOfDisplays; ++i) {
        SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, nullptr };
        auto mode_index = 0;
        if(SDL_GetDisplayMode(i, mode_index, &mode) != 0) {
            LOG(ERROR) << "SDL_GetDisplayMode failed:" << SDL_GetError();
            continue;
        }
        LOG(INFO) << "DisplayMode " << SDL_BITSPERPIXEL(mode.format) << " " << mode.w << " x "
                  << mode.h << " " << SDL_GetPixelFormatName(mode.format);
    }

    auto glmaVersion = 0;
    auto glmiVersion = 0;
    if(SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glmaVersion) != 0) {
        glmaVersion = 0;
        LOG(ERROR) << "SDL_GL_GetAttribute failed:" << SDL_GetError();
    }

    if(SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glmiVersion) != 0) {
        glmiVersion = 0;
        LOG(ERROR) << "SDL_GL_GetAttribute failed:" << SDL_GetError();
    }

    LOG(INFO) << "Open GL Version " << glmaVersion << "." << glmiVersion;

    SDL_version compiled;
    SDL_IMAGE_VERSION(&compiled);
    const auto linked = *IMG_Linked_Version();

    LOG(INFO) << "SDL_image Version compiled " << static_cast<int>(compiled.major) << "."
              << static_cast<int>(compiled.minor) << "." << static_cast<int>(compiled.patch);
    LOG(INFO) << "SDL_image Version linked " << static_cast<int>(linked.major) << "."
              << static_cast<int>(linked.minor) << "." << static_cast<int>(linked.patch);

    const auto flags = IMG_INIT_PNG | IMG_INIT_WEBP | IMG_INIT_JPG;
    const auto initted = IMG_Init(flags);
    if((initted & flags) != flags) {
        throw SDLException("InitImg");
    }

    float ddpi, hdpi, vdpi;

    if(SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
        // This happend on Oracle VirtualBox and on PI
        LOG(ERROR) << "get dpi failed " << SDL_GetError();
        ddpi = 96.0;
    }

    return ddpi;
}

/**
 * @brief Starting SDL2 Audio Subsystem
 * @param[in] drivername the AudioDriver or emtpy than we search for driver
 * @return the success
 */
bool SDLBase::InitAudio(const std::string& drivername)
{
    if(InitSubsystem(SDL_INIT_AUDIO) < 0) {
        throw SDLException("InitAudio");
    }

    auto count = SDL_GetNumAudioDevices(0);
    if(count == 0) {
        LOG(WARNING) << "Warning: no Audio device found";
        initAudioDone_ = false;
        return false;
    }
 
    for(auto i = 0; i < count; ++i) {
        const char *devname = SDL_GetAudioDeviceName(i, 0);
        LOG(INFO) << "Audio device " << i << " " << devname;
    }

    initAudioDone_ = true;
    if(!drivername.empty()) {
        for(auto i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
            std::string drivernameFound = SDL_GetAudioDriver(i);
            LOG(INFO) << "Audio Driver found " << drivernameFound;
            if(drivernameFound == drivername) {
                if(SDL_AudioInit(drivername.c_str()) < 0) {
                    initAudioDone_ = false;
                    throw SDLException("SDL_AudioInit");
                } else {
                    break;
                }
            }
        }
    }

    std::string usedDriver = SDL_GetCurrentAudioDriver();
    if(usedDriver != drivername && !drivername.empty()) {
        LOG(WARNING) << "Configure Audio Driver not found";
    }

    LOG(INFO) << "Using audio driver: " << usedDriver;

    return true;
}

int SDLBase::InitSubsystem(uint32_t flags)
{
    auto result = SDL_WasInit(SDL_INIT_EVERYTHING);
    if(result & flags) {
        return 0;
    }

    if(SDL_InitSubSystem(flags) < 0) {
        throw SDLException("InitSubSystem");
    }

    LogSystemsRunning();

    return 0;
}

/**
 * @brief Callback Funktion redirect SDL LogEntry to Mainlog
 * @param[in] userdata pointer of @class SDLBase
 * @param[in] category https://wiki.libsdl.org/CategoryLog
 * @param[in] priority
 * @param[in] message
 */
void SDLBase::SdlLogEntry(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
    if(VLOG_IS_ON(2)) {
        auto kernel = reinterpret_cast<SDLBase*>(userdata);

        std::string categoryText = "";
        switch(category) {
        case SDL_LOG_CATEGORY_APPLICATION: {
            categoryText = "Application ";
            break;
        }

        case SDL_LOG_CATEGORY_ERROR: {
            categoryText = "Error ";
            break;
        }

        case SDL_LOG_CATEGORY_SYSTEM: {
            categoryText = "System ";
            break;
        }

        case SDL_LOG_CATEGORY_AUDIO: {
            categoryText = "Audio ";
            break;
        }
        case SDL_LOG_CATEGORY_VIDEO: {
            categoryText = "Video ";
            break;
        }
        case SDL_LOG_CATEGORY_RENDER: {
            categoryText = "Render ";
            break;
        }
        case SDL_LOG_CATEGORY_INPUT: {
            categoryText = "Input ";
            break;
        }
        default: {
            categoryText = "Unkwon ";
        }
        }

        auto logText = categoryText;
        logText += " ";
        logText += message;
        switch(priority) {
        case SDL_LOG_PRIORITY_VERBOSE:
            CVLOG(1, "SDL") << logText;
            break;
        case SDL_LOG_PRIORITY_DEBUG:
            LOG(DEBUG) << logText;
            break;
        case SDL_LOG_PRIORITY_INFO:
            CLOG(INFO, "SDL") << logText;
            break;
        case SDL_LOG_PRIORITY_WARN:
            CLOG(WARNING, "SDL") << logText;
            break;
        case SDL_LOG_PRIORITY_ERROR: {
            CLOG(ERROR, "SDL") << logText;
            if(kernel) {
                // Todo: kernel->ShowErrorOnScreen(message);
            }
            break;
        }
        case SDL_LOG_PRIORITY_CRITICAL: {
            CLOG(FATAL, "SDL") << logText;
            if(kernel) {
                // Todo: kernel->ShowErrorOnScreen(message);
            }
            break;
        }
        default:
            CLOG(ERROR, "SDL") << "SDL priority not implemented " << logText;
            break;
        }
    }
}

void SDLBase::LogSystemsRunning()
{
    auto subsystem_init = SDL_WasInit(SDL_INIT_EVERYTHING);
    if(subsystem_init & SDL_INIT_VIDEO) {
        LOG(INFO) << "Video is running";
    }

    if(subsystem_init & SDL_INIT_AUDIO) {
        LOG(INFO) << "Audio is running";
    }

    if(subsystem_init & SDL_INIT_TIMER) {
        LOG(INFO) << "Timer is running";
    }

    if(subsystem_init & SDL_INIT_JOYSTICK) {
        LOG(INFO) << "JOYSTICK is running";
    }

    if(subsystem_init & SDL_INIT_HAPTIC) {
        LOG(INFO) << "HAPTIC is running";
    }

    if(subsystem_init & SDL_INIT_GAMECONTROLLER) {
        LOG(INFO) << "GAMECONTROLLER is running";
    }

    if(subsystem_init & SDL_INIT_EVENTS) {
        LOG(INFO) << "EVENTS is running";
    }
}
