#pragma once

#include <exception>
#include <string>

class NullPointerException : public std::exception
{
public:
	explicit NullPointerException(const char* message);
	virtual ~NullPointerException() throw();

	const char* what() const throw() override
	{
		return mMessage.c_str();
	}

protected:
	std::string mMessage;

private:
	NullPointerException();
};
