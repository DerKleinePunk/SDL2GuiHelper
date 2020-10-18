#pragma once
#include <functional>
#include <SDL.h>

namespace utils
{
    typedef std::function<void(void *userdata, int category, SDL_LogPriority priority, const char *message)> LOGCALLBACK;
    void LogOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message);
}