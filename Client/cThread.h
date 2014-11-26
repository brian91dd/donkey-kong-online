#ifndef cThreadH
#define cThreadH

#include <pthread.h>

class cThread
{
  public:
    cThread();
    int start( void* args );
    int join();
    int detach();
    void cancel();
    virtual ~cThread();

  protected:
    static void* runProcess( void* );
    virtual int process( void* ) = 0;

  private:
    bool engaged;
    bool detached;
    pthread_t id;
    void* args;
};

#endif
