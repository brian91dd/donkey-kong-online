#ifndef cServerConnH
#define cServerConnH

#include "cThread.h"
#include "cTCPSocket.h"
#include "cMutex.h"

class cServerConn : public cThread
{
  public:
    cTCPSocket sd;
    cServerConn();
    char get_status();
    void set_status( char status );
    void send( std::string msg );
    bool is_connected();
    void disconnect();
    void connect( std::string ip, int port, bool reconect = false );
    virtual ~cServerConn();
    virtual int process( void* );
    static const char NOT_DEFINED = 0;
    static const char CONNECTED = 1;
    static const char NOT_CONNECTED = 2;
    static const char NO_HOST = 3;
    static const char DISCONNECTED = 4;

  private:
    char status;
    cMutex m_status;
    static const int BUFFERSIZE = 1024;
    cServerConn( const cServerConn& );
    cServerConn& operator=( const cServerConn& );
};

#endif
