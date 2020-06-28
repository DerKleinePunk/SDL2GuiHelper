#include "SqliteException.h"
#include <sqlite3.h>
#include "../easylogging/easylogging++.h"

SqliteException::SqliteException() {
}

SqliteException::SqliteException(const char* message) {
	mMessage = message;
	LOG(ERROR) << "SqliteException " << message;
}

SqliteException::SqliteException(char* message, const bool bDeleteMsg/*=false*/) {
	mMessage = message;
	LOG(ERROR) << "SqliteException " << message;
	if(bDeleteMsg) {
		sqlite3_free(message);
	}
}

SqliteException::SqliteException(const std::string& message) {
	mMessage = message;
}

SqliteException::~SqliteException() throw() {
}
