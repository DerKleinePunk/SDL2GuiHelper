#include "NullPointerException.h"
#include "../easylogging/easylogging++.h"

NullPointerException::NullPointerException()
{
}

NullPointerException::NullPointerException(const char* message) : _message(message)
{
    LOG(ERROR) << "NullPointerException " << message;
}

NullPointerException::~NullPointerException() throw()
{
}