#pragma once

#include <exception>
#include <string>

class NotImplementedException : public std::exception
{
  public:
    explicit NotImplementedException(const char* message);
    virtual ~NotImplementedException() throw();

    const char* what() const throw() override
    {
        return _message.c_str();
    }

  protected:
    std::string _message;

  private:
    NotImplementedException();
};
