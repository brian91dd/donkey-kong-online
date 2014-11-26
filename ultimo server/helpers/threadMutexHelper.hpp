#ifndef _H_THREADS_MUTEX_HELPER
	#define _H_THREADS_MUTEX_HELPER

	#include <pthread.h>
	#include <map>
	#include <iostream>
	using namespace std;
	class ThreadMutexHelper
	{
	  public:
		ThreadMutexHelper();
		int Lock(string);
		int Unlock(string);
		void createMutex(string);
		void close(string);
		void close();

	  private:
		map<string,pthread_mutex_t> mutexes;
	};

#endif
