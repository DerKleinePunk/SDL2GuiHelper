#include "IllegalStateException.h"
#include "../easylogging/easylogging++.h"

IllegalStateException::IllegalStateException() {
}

IllegalStateException::IllegalStateException(const char* message):_message(message) {
	LOG(ERROR) << "NullPointerException " << message;
}

IllegalStateException::~IllegalStateException() throw() {
}