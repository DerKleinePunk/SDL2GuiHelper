#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "SDLException"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "SDLException.h"
#include <SDL.h>
#include "../../common/easylogging/easylogging++.h"

std::string SDLException::CreateText(const std::string& function, const std::string& errormsg)
{
    auto tmp(function);
    tmp += " failed with: ";
    tmp += errormsg;
    return tmp;
}

SDLException::SDLException(const char* function)
    : sdl_function_(function), sdl_error_(SDL_GetError())
{
    message_ = CreateText(sdl_function_, sdl_error_);
    LOG(ERROR) << "SDLException " << message_;
}

SDLException::SDLException(const std::string& function)
    : sdl_function_(function), sdl_error_(SDL_GetError())
{
    message_ = CreateText(sdl_function_, sdl_error_);
    LOG(ERROR) << "SDLException " << message_;
}

SDLException::~SDLException() throw()
{
}
