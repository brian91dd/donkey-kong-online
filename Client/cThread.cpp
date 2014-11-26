#include "cThread.h"
#include <pthread.h>

cThread::cThread(): engaged( false ), detached( false ){}

int cThread::start( void* args )
{
  this->args = args;
  int error = pthread_create( &( this->id ), NULL, &( cThread::runProcess ), this );
  if( !error ) this->engaged = true;
  return error;
}

int cThread::join()
{
  int res = 0;
  if( !this->detached ) res = pthread_join( this->id, NULL );
  return res;
}

/*
runProcess() tiene que ser del tipo [void* func(void*)] para poder ponerlo como
parametro en el pthread_create(). Luego, este llamara a Process() que es un metodo
virtual que definiran las clases que hereden cThread.
*/
void* cThread::runProcess( void* thread )
{
  cThread* thisThread = ( cThread* ) thread;
  if ( thisThread )
  {
    thisThread->process( thisThread->args );
  }
  return 0;
}

int cThread::detach()
{
  int res = 0;
  if( this->engaged && !this->detached )
  {
    res = pthread_detach( this->id );
    if( res == 0 ) this->detached = true;
  }
  return res;
}

void cThread::cancel()
{
  if( this->engaged ) pthread_cancel( this->id );
}

cThread::~cThread()
{
}
