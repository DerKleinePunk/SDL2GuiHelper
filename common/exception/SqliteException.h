#pragma once

#include <exception>
#include <string>

static const bool DELETE_MSG = true;

class SqliteException : public std::exception
{
public:
	explicit SqliteException(const char* message);
	explicit SqliteException(char* message, bool bDeleteMsg = false);
	explicit SqliteException(const std::string& message);
	virtual ~SqliteException() throw();

	const char* what() const throw() override
	{
		return mMessage.c_str();
	}

protected:
	std::string mMessage;

private:
	SqliteException();
};