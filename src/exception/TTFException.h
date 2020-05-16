#pragma once

class TTFException : public std::exception
{
private:
	std::string sdl_function_;
	std::string sdl_error_;
	std::string message_;

	static std::string CreateText(std::string function, std::string errormsg);

public:
	explicit TTFException(const char* function);
	explicit TTFException(const std::string& function);
	virtual ~TTFException() throw();

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
