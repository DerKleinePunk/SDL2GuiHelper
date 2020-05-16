#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "SDLException"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include "SDLException.h"
#include <SDL.h>

std::string SDLException::CreateText(std::string function, std::string errormsg)
{
	auto tmp(function);
	tmp += " failed with: ";
	tmp += errormsg;
	return tmp;
}	

SDLException::SDLException(const char* function)
{
	sdl_error_ = SDL_GetError();
	sdl_function_ = function;
	message_ = CreateText(sdl_function_, sdl_error_);
    LOG(ERROR) << "SDLException " << message_;
}

SDLException::SDLException(const std::string& function)
{
	sdl_error_ = SDL_GetError();
	sdl_function_ = function;
	message_ = CreateText(sdl_function_, sdl_error_);
    LOG(ERROR) << "SDLException " << message_;
}

SDLException::~SDLException() throw()
{

}
