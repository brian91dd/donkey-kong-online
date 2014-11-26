#ifndef cSocketExceptionH
#define cSocketExceptionH

#include <exception>
#include <string>

class cSocketException : public std::exception
{
  public:
    cSocketException( const std::string &error ) throw();
    ~cSocketException() throw();
    const char* what() const throw();

  private:
    std::string error;
};

#endif
