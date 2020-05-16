#pragma once

#include <exception>
#include <string>

class ArgumentException: public std::exception
{
public:
	explicit ArgumentException(const char* message);
	virtual ~ArgumentException() throw();

	const char* what() const throw() override
	{
		return _message.c_str();
	}

protected:
	std::string _message;

private:
	ArgumentException();
};
