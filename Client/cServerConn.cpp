#include "cServerConn.h"

#include "cInstruction.h"
#include "cTCPSocket.h"
#include "cSocketException.h"
#include "cSafeQueue.h"
#include "cLockableGuard.h"

#include <iostream>
#include <string>
#include <fstream>
#include <string.h>

using namespace std;

cServerConn::cServerConn() : status(cServerConn::NOT_DEFINED)
{
}

void cServerConn::connect( string ip, int port, bool reconnect )
{
  int reconnect_tries = 0;
  this->set_status( cServerConn::NOT_CONNECTED );
  do
  {
    try
    {
      printf( "Connecting to:\n" );
      printf( " - IP: %s\n", ip.c_str() );
      printf( " - Port: %d\n", port );
      this->sd.connect( ip, port );
      this->set_status( cServerConn::CONNECTED );
      printf( "Connected.\n" );
    }
    catch( cSocketException &e )
    {
      cerr << e.what() << endl;
      this->set_status( cServerConn::NO_HOST );
      reconnect_tries++;
      if( reconnect )
      {
        if( reconnect_tries == 10 ) return;
        sleep(1);
      }
    }
  } while( reconnect && this->get_status() == cServerConn::NO_HOST );
}

int cServerConn::process( void* args )
{
  cSafeQueue<cInstruction>* messages = ( cSafeQueue<cInstruction>* ) args;
  try
  {
    string msg = "";
    char buffer[cServerConn::BUFFERSIZE];
    memset( buffer, 0, cServerConn::BUFFERSIZE );
    string buffer_str = "";
    unsigned int i;

    while( this->get_status() == cServerConn::CONNECTED )
    {
      this->sd.receive( buffer, cServerConn::BUFFERSIZE );
      buffer_str = buffer;
      while( ( i = buffer_str.find( "|" ) ) != (unsigned int)string::npos )
      {
        msg = buffer_str.substr( 0, i );
        buffer_str = buffer_str.substr( i + 1 );
        //printf( "Recv: %s\n", msg.c_str() );
        cInstruction instruction;
        if( msg.find(" ") )
        {
          string action = msg.substr (0, msg.find(" ") );
          instruction.set_action( action );
          string params = msg.substr( msg.find(" ") + 1 );
          unsigned int pos;
          while( ( pos = params.find(";") ) != (unsigned int)string::npos )
          {
            instruction.add_param( params.substr( 0, pos ) );
            params = params.substr( pos + 1 );
          }
          instruction.add_param( params );
        }
        else
        {
          instruction.set_action( msg );
        }
        messages->push( instruction );
        #ifdef SOCK_TO_TXT
        std::ofstream outfile( "recv_queue.txt", std::ofstream::out | std::ofstream::app );
        outfile.write( inst_str.c_str(), inst_str.size() );
        outfile.write( "\n", 1 );
        outfile.close();
        #endif
      }
      memset( buffer, 0, cServerConn::BUFFERSIZE );
    }
  }
  catch( cSocketException &e )
  {
    cerr << e.what() << endl;
    this->set_status( cServerConn::NO_HOST );
  }
  return 0;
}

void cServerConn::send( string msg )
{
  if( this->get_status() == cServerConn::CONNECTED )
  {
    try
    {
      printf( "Send: %s\n", msg.c_str() );
      string tmp = msg  + "|";
      this->sd.send( tmp.c_str(), tmp.length() );
    }
    catch( cSocketException &e )
    {
      cerr << e.what() << endl;
      this->set_status( cServerConn::NOT_CONNECTED );
    }
  }
}

bool cServerConn::is_connected()
{
  return !this->sd.is_closed();
}

char cServerConn::get_status()
{
  cLockableGuard<cMutex, int, int> safe_mutex( this->m_status, &cMutex::Lock, &cMutex::Unlock );
  return this->status;
}

void cServerConn::set_status( char status )
{
  cLockableGuard<cMutex, int, int> safe_mutex( this->m_status, &cMutex::Lock, &cMutex::Unlock );
  this->status = status;
}

void cServerConn::disconnect()
{
  this->set_status( cServerConn::DISCONNECTED );
  // envio un shutdown al socket para que se destrabe el receive
  this->sd.shutdown();
  // cierro el socket
  this->sd.close();
}

cServerConn::~cServerConn()
{
  this->set_status( cServerConn::NOT_CONNECTED );
  // envio un shutdown al socket para que se destrabe el receive
  this->sd.shutdown();
  // cierro el socket
  this->sd.close();
}
