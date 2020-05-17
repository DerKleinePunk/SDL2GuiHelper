#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "TTFException"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../../common/easylogging/easylogging++.h"
#include <SDL_ttf.h>
#include "TTFException.h"

std::string TTFException::CreateText(std::string function, std::string errormsg)
{
	auto tmp(function);
	tmp += " failed with: ";
	tmp += errormsg;
	return tmp;
}	

TTFException::TTFException(const char* function) {
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	sdl_error_ = TTF_GetError();
	sdl_function_ = function;
	message_ = CreateText(sdl_function_, sdl_error_);
	LOG(ERROR) << "TTFException " << message_;
}

TTFException::TTFException(const std::string& function) {
	el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	sdl_error_ = TTF_GetError();
	sdl_function_ = function;
	message_ = CreateText(sdl_function_, sdl_error_);
	LOG(ERROR) << "TTFException " << message_;
}

TTFException::~TTFException() throw()
{

}
