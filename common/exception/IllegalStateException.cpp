#include "IllegalStateException.h"
#include "../easylogging/easylogging++.h"

IllegalStateException::IllegalStateException() {
}

IllegalStateException::IllegalStateException(const char* message) {
	mMessage = message;
	LOG(ERROR) << "NullPointerException " << message;
}

IllegalStateException::~IllegalStateException() throw() {
}