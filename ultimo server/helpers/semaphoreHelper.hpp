#include <stdio.h>
#include <iostream>
#include <semaphore.h>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#ifndef _H_SEMAPHORE_HELPER
#define _H_SEMAPHORE_HELPER
	using namespace std;
	class SemaphoreHelper {
		private:
			map<string,sem_t *> mapName;
		public:
			SemaphoreHelper();
			bool createSemaphore(string,int);
			bool connectToSemaphore(string);
			void P(string);
			void V(string);
			void set(string,sem_t *);
			int getValue(string);
			sem_t* get(string);
			void close();
	};
#endif
