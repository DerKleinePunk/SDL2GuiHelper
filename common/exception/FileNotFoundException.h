#pragma once

#include <exception>
#include <string>

class FileNotFoundException : public std::exception
{
public:
	explicit FileNotFoundException(const char* message);
	virtual ~FileNotFoundException() throw();

	const char* what() const throw() override
	{
		return _message.c_str();
	}

protected:
	std::string _message;

private:
	FileNotFoundException();
};
