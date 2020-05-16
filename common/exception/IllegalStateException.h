#pragma once

#include <exception>
#include <string>

class IllegalStateException : public std::exception
{
public:
	explicit IllegalStateException(const char* message);
	virtual ~IllegalStateException() throw();

	const char* what() const throw() override
	{
		return mMessage.c_str();
	}

protected:
	std::string mMessage;

private:
	IllegalStateException();
};
