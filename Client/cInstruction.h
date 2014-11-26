#ifndef cInstructionH
#define cInstructionH

#include <string>
#include <vector>

class cInstruction
{
  public:
    void set_action( std::string msg );
    void add_param( std::string param );
    std::string get_action() const;
    std::string get_param( int index ) const;
    unsigned int size_params() const;
    std::vector<std::string> split( const std::string &s, char delim );

  private:
    std::string action;
    std::vector<std::string> params;
};

#endif

