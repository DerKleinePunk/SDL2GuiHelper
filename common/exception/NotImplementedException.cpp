#include "NotImplementedException.h"
#include "../easylogging/easylogging++.h"

NotImplementedException::NotImplementedException() {
}

NotImplementedException::NotImplementedException(const char* message) {
	mMessage = message;
	LOG(ERROR) << "NotImplementedException " << message;
}

NotImplementedException::~NotImplementedException() throw() {
}
