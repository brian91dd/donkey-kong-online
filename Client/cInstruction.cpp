#include "cInstruction.h"
#include <string>
#include <sstream>

using namespace std;

void cInstruction::set_action( std::string msg )
{
  this->action = msg;
}

void cInstruction::add_param( std::string param )
{
  this->params.push_back( param );
}

string cInstruction::get_action() const
{
  return this->action;
}

string cInstruction::get_param( int i ) const
{
  return this->params[i-1];
}

vector<string> cInstruction::split( const string &s, char delim )
{
  stringstream ss(s);
  string item;
  vector<string> elems;
  while( getline( ss, item, delim ) ) elems.push_back( item );
  return elems;
}

unsigned int cInstruction::size_params() const
{
  return this->params.size();
}
