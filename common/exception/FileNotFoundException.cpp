#include "FileNotFoundException.h"
#include "../easylogging/easylogging++.h"

FileNotFoundException::FileNotFoundException() {
}

FileNotFoundException::FileNotFoundException(const char* message):_message(message) {
	LOG(ERROR) << "FileNotFoundException " << message;
}

FileNotFoundException::~FileNotFoundException() throw() {
}