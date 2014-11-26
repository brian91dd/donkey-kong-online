#ifndef cSharedMemoryH
#define cSharedMemoryH

#include <cstddef>
#include <string>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>

template <typename T>
class cSharedMemory
{
  public:
		cSharedMemory( std::string name, int cant = 1 )
		{
			this->cant = cant;

			// En base al nombre genero una key de tipo int
			int key = this->hash( name.c_str() );

			// Crea una semaforo SYS V que luego se va a usar para poder acceder a la
			// memoria compartida de manera sincronizada mediante los metodos de la clase
			this->sem = semget( ftok( ".", key ), 1, 0666 | IPC_CREAT );
			union semun {
				int val;
				struct semid_ds *buf;
				ushort *array;
			} semctl_arg;
			semctl_arg.val = 1;
			semctl( this->sem, 0, SETVAL, semctl_arg );

			// Crea la memoria compartida
			this->shmid = shmget( ftok( ".", key + 1 ), sizeof(T) * cant, 0666 | IPC_CREAT ) ;
			this->data = (T*) shmat( this->shmid, NULL, 0 );
		}

		T get( int key = 0 )
		{
			this->p();
			T ret = this->data[key];
			this->v();
			return ret;
		}

		void set( int key, T value )
		{
			this->p();
			this->data[key] = value;
			this->v();
		}

		void set( T value )
		{
			this->set( 0, value );
		}

		int size()
		{
			return cant;
		}

		~cSharedMemory()
		{
			// Lo marca para que se elimine cuando ya no tenga ningun proceso attached
			shmctl( this->shmid, IPC_RMID, NULL );
			// Obtengo cuantos procesos estan attached ahora
			struct shmid_ds shmds;
			shmctl( this->shmid, IPC_STAT, &shmds );
			shmdt( (void*) this->data );
			// Si solo habia 1 proceso attached (el ultimo que quedaba) borro el semaforo
			if( shmds.shm_nattch == 1 ) semctl( this->sem, 0, IPC_RMID );
		}


  private:
    int sem;
    int shmid;
    T* data;
    int cant;
		void p()
		{
			struct sembuf opts_sem;
			opts_sem.sem_num = 0;
			opts_sem.sem_op = -1;
			opts_sem.sem_flg = 0;
			semop( this->sem, &opts_sem, 1 );
		}
		void v()
		{
			struct sembuf opts_sem;
			opts_sem.sem_num = 0;
			opts_sem.sem_op = 1;
			opts_sem.sem_flg = 0;
			semop( this->sem, &opts_sem, 1 );
		}
		int hash( const char *str )
		{
			int hash = 5381;
			int c;
			while( ( c = *str++ ) ) hash = ( ( hash << 5 ) + hash ) + c;
			return hash;
		}
};

#endif
