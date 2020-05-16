#include <SDL.h>
#include "GUIException.h"
#include "../../common/easylogging/easylogging++.h"


std::string GUIException::CreateText(std::string function, std::string errormsg)
{
	auto tmp(function);
	tmp += " failed with: ";
	tmp += errormsg;
	return tmp;
}	

GUIException::GUIException(const char* function) {
	sdl_error_ = SDL_GetError();
	sdl_function_ = function;
	message_ = CreateText(sdl_function_, sdl_error_);
    LOG(ERROR) << "GUIException " << message_;
}

GUIException::GUIException(const std::string& function) {
	sdl_error_ = SDL_GetError();
	sdl_function_ = function;
	message_ = CreateText(sdl_function_, sdl_error_);
    LOG(ERROR) << "GUIException " << message_;
}

GUIException::~GUIException() throw()
{

}
