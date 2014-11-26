#ifndef cLockableGuardH
#define cLockableGuardH

template<class LockableObject, typename retLock, typename retUnlock>
class cLockableGuard
{
  private:
    bool locked;
    LockableObject& object;
    retLock( LockableObject::*funcLock )();
    retUnlock( LockableObject::*funcUnlock )();
    cLockableGuard( const cLockableGuard& );
    cLockableGuard& operator=( const cLockableGuard& );

  public:
    cLockableGuard( LockableObject& object, retLock( LockableObject::*funcLock )(), retUnlock( LockableObject::*funcUnlock )() ): locked( true ), object( object ), funcLock( funcLock ), funcUnlock( funcUnlock )
    {
      Lock();
    }

    void Lock()
    {
      retLock error = ( object.*funcLock )();
      if( !error ) locked = true;
    }

    void Unlock()
    {
      retUnlock error = ( object.*funcUnlock )();
      if( !error ) locked = false;
    }

    ~cLockableGuard()
    {
      if( locked )( object.*funcUnlock )();
    }
};

#endif
