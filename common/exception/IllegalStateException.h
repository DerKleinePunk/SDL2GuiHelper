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
		return _message.c_str();
	}

protected:
	std::string _message;

private:
	IllegalStateException();
};
