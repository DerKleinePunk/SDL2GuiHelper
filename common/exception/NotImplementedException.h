#pragma once

#include <exception>
#include <string>

class NotImplementedException: public std::exception
{
public:
	explicit NotImplementedException(const char* message);
	virtual ~NotImplementedException() throw();

	const char* what() const throw() override
	{
		return mMessage.c_str();
	}

protected:
	std::string mMessage;

private:
	NotImplementedException();
};
