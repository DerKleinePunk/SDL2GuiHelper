#pragma once
#include <exception>
#include <string>

class SDLException : public std::exception
{
private:
	std::string sdl_function_;
	std::string sdl_error_;
	std::string message_;

	static std::string CreateText(const std::string& function, const std::string& errormsg);

public:
	explicit SDLException(const char* function);
	explicit SDLException(const std::string& function);
	virtual ~SDLException() throw();

	const char* what() const throw() override
	{
		return message_.c_str();
	}

	const char* Function() const throw()
	{
		return sdl_function_.c_str();
	}

	const char* Error() const throw()
	{
		return sdl_error_.c_str();
	}
};
