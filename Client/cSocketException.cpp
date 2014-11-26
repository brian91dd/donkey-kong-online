#include "cSocketException.h"
#include <string>
#include <sstream>

#include <errno.h>
#include <string.h>

using namespace std;

cSocketException::cSocketException( const string &error ) throw(): error( error )
{
  this->error.append( ": " );
  this->error.append( strerror( errno ) );
}

const char *cSocketException::what() const throw()
{
  return this->error.c_str();
}

cSocketException::~cSocketException() throw() {}
