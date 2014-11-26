#ifndef cMutexH
#define cMutexH

#include <pthread.h>

class cMutex
{
  public:
    cMutex();
    int Lock();
    int Unlock();
    ~cMutex();

  private:
    pthread_mutex_t mutex;
    cMutex( const cMutex& );
    cMutex &operator=( const cMutex& );
};

#endif
