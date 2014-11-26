#include "cMutex.h"

cMutex::cMutex()
{
  pthread_mutex_init( &( this->mutex ), NULL );
}

int cMutex::Lock()
{
  return pthread_mutex_lock( &( this->mutex ) );
}

int cMutex::Unlock()
{
  return pthread_mutex_unlock( &( this->mutex ) );
}

cMutex::~cMutex()
{
  pthread_mutex_destroy( &( this->mutex ) );
}
