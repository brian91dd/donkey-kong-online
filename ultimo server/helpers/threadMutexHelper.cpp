#include "threadMutexHelper.hpp"

ThreadMutexHelper::ThreadMutexHelper() {

}
void ThreadMutexHelper::createMutex(string index) {
	pthread_mutex_t mutex;
	pthread_mutex_init( &mutex, NULL );
	this->mutexes[index] = mutex;
}

int ThreadMutexHelper::Lock(string index) {
	return pthread_mutex_lock( &this->mutexes[index] );
}

int ThreadMutexHelper::Unlock(string index) {
	return pthread_mutex_unlock( &this->mutexes[index] );
}

void ThreadMutexHelper::close(string index) {
	pthread_mutex_destroy( &this->mutexes.find(index)->second );
}

void ThreadMutexHelper::close() {
	for (std::map<string,pthread_mutex_t>::iterator it=this->mutexes.begin(); it!=this->mutexes.end(); ++it) {
		pthread_mutex_destroy( &it->second );
	}
}