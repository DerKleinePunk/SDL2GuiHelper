#include "../easylogging/easylogging++.h"
#include "ArgumentException.h"


ArgumentException::ArgumentException() {
}

ArgumentException::ArgumentException(const char* message):
	_message(message){
	LOG(ERROR) << "ArgumentException " << message;
}

ArgumentException::~ArgumentException() throw() {
}