#include "cTCPSocket.h"
#include "cSocketException.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>

// Si defino SOCK_TO_TXT todo lo que entre y salga por los sockets tambien
// sera escrito en dos archivos de texto: send.txt y recv.txt (debugging)
//#define SOCK_TO_TXT

using namespace std;

sockaddr_in cTCPSocket::getAddressStruct( const string& address, unsigned int port )
{
  sockaddr_in strctAddr;
  memset( &strctAddr, 0, sizeof( address ) );

  hostent* he;
  if( ( he = gethostbyname( address.c_str() ) ) == NULL )
  {
    throw cSocketException( "Error en gethostbyname()" );
  }

  strctAddr.sin_family = AF_INET;
  strctAddr.sin_addr = *(( struct in_addr * )he->h_addr );
  strctAddr.sin_port = htons( port );

  return strctAddr;
}

cTCPSocket::cTCPSocket()
{
  if( ( sockDesc = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
  {
    throw cSocketException( "Error en socket()" );
  }
#ifdef SOCK_TO_TXT
#ifndef SOCK_TO_TXT_INIT
#define SOCK_TO_TXT_INIT
  std::ofstream outfile;
  outfile.open( "send.txt", std::ofstream::out | std::ofstream::trunc );
  outfile.close();
  outfile.open( "recv.txt", std::ofstream::out | std::ofstream::trunc );
  outfile.close();
#endif
#endif
}

cTCPSocket::cTCPSocket( int sockDesc )
{
  this->sockDesc = sockDesc;
}

void cTCPSocket::connect( const string& host, unsigned int port )
{
  try
  {
    sockaddr_in destAddr = getAddressStruct( host, port );

    if( ::connect( sockDesc, ( sockaddr* ) &destAddr, sizeof( destAddr ) ) == -1 )
    {
      throw cSocketException( "Error en connect()" );
    }
  }
  catch( cSocketException &e )
  {
    throw e;
  }
}

void cTCPSocket::listen( unsigned int port, unsigned int backlog )
{
  sockaddr_in localAddr;

  memset( &localAddr, 0, sizeof( localAddr ) );
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
  localAddr.sin_port = htons( port );

  if( bind( sockDesc, ( sockaddr* ) &localAddr, sizeof( localAddr ) ) == -1 )
  {
    throw cSocketException( "Error en bind()" );
  }

  if( ::listen( sockDesc, backlog ) == -1 )
  {
    throw cSocketException( "Error en listen()" );
  }
}

void cTCPSocket::send( const char* stream, unsigned int size )
{
  if( ::send( sockDesc, ( void* ) stream, size, MSG_NOSIGNAL ) == -1 )
  {
    throw cSocketException( "Error en send()" );
  }
#ifdef SOCK_TO_TXT
  std::ofstream outfile( "send.txt", std::ofstream::out | std::ofstream::app );
  outfile.write( stream, size );
  outfile.close();
#endif
}

int cTCPSocket::receive( char* stream, unsigned int size )
{
  int ret;
  if( ( ret = recv( sockDesc, ( void* ) stream, size, 0 ) ) == -1 )
  {
    throw cSocketException( "Error en recv()" );
  }
#ifdef SOCK_TO_TXT
  std::ofstream outfile( "recv.txt", std::ofstream::out | std::ofstream::app );
  outfile.write( stream, ret );
  outfile.close();
#endif
  return ret;
}

cTCPSocket* cTCPSocket::accept()
{
  int newSockDesc;
  if( ( newSockDesc = ::accept( sockDesc, NULL, 0 ) ) == -1 )
  {
    throw cSocketException( "Error en accept()" );
  }

  return new cTCPSocket( newSockDesc );
}

bool cTCPSocket::is_closed()
{
  fd_set rfd;
  FD_ZERO( &rfd );
  FD_SET( this->sockDesc, &rfd );
  timeval tv = { 0 };
  select( this->sockDesc+1, &rfd, 0, 0, &tv );
  if( !FD_ISSET( this->sockDesc, &rfd ) ) return false;
  int n = 0;
  ioctl( this->sockDesc, FIONREAD, &n );
  return n == 0;
}

void cTCPSocket::shutdown()
{
  ::shutdown( sockDesc, SHUT_RDWR );
}

void cTCPSocket::close()
{
  if( sockDesc != -1 )
  {
    ::close( sockDesc );
    sockDesc = -1;
  }
}

cTCPSocket::~cTCPSocket()
{
  this->shutdown();
  this->close();
}

