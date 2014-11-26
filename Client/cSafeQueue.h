#ifndef cSafeQueueH
#define cSafeQueueH

#include "cMutex.h"
#include "cLockableGuard.h"
#include <queue>

template <typename T>
class cSafeQueue
{
  private:
    cMutex mutex;
    std::queue<T> cola;
    cSafeQueue( const cSafeQueue& );
    cSafeQueue &operator=( const cSafeQueue& );

  public:
    cSafeQueue(){};

    void push( T element )
    {
      cLockableGuard<cMutex, int, int> safe_mutex( this->mutex, &cMutex::Lock, &cMutex::Unlock );
      this->cola.push( element );
    }

    T pop()
    {
      cLockableGuard<cMutex, int, int> safe_mutex( this->mutex, &cMutex::Lock, &cMutex::Unlock );
      T ret = cola.front();
      this->cola.pop();
      return ret;
    }

    int size()
    {
      cLockableGuard<cMutex, int, int> safe_mutex( this->mutex, &cMutex::Lock, &cMutex::Unlock );
      return cola.size();
    }

    bool is_empty()
    {
      cLockableGuard<cMutex, int, int> safe_mutex( this->mutex, &cMutex::Lock, &cMutex::Unlock );
      return cola.empty();
    }
};

#endif
