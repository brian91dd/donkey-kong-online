#include "semaphoreHelper.hpp"


SemaphoreHelper::SemaphoreHelper() {

}

bool SemaphoreHelper::createSemaphore(string index,int val) {
	string semName = index;
	if ( this->mapName.find(index) == this->mapName.end() ) {
		sem_t *sem = sem_open(semName.c_str(), O_CREAT|O_EXCL, 0644, val);
		if(sem == SEM_FAILED) {
			if(errno == EEXIST) {
				return false;
			} else {
		        perror("child sem_open");
		        exit(1);
		        return false;
			}
	    }
		this->mapName[index] = sem;
		return true;
	} else {
		return false;
	}
}

bool SemaphoreHelper::connectToSemaphore(string index) {
	string semName = index;
	if ( this->mapName.find(index) == this->mapName.end() ) {
		sem_t *sem = sem_open(semName.c_str(), O_CREAT,0);
		if(sem == SEM_FAILED) {
	        perror("child sem_open");
	    }
		this->mapName[index] = sem;
		return true;
	} else {
		return false;
	}
}

void SemaphoreHelper::set(string index,sem_t* sem) {
	this->mapName[index] = sem;
	//this->mapName.insert(index,num);
}

void SemaphoreHelper::P(string sem) {
	if ( this->mapName.find(sem) != this->mapName.end() ) {
		sem_wait(this->mapName.find(sem)->second);
	} else {
		cout << "No existe el semaforo" << endl;
	}
}

void SemaphoreHelper::V(string sem) {
	if ( this->mapName.find(sem) != this->mapName.end() ) {
		sem_post(this->mapName.find(sem)->second);
	} else {
		cout << "No existe el semaforo" << endl;
	}

}

int SemaphoreHelper::getValue(string sem) {
	int value;
	sem_getvalue(this->mapName.find(sem)->second, &value);
}

sem_t* SemaphoreHelper::get(string index) {
	return this->mapName.find(index)->second;
}

void SemaphoreHelper::close() {
	for (std::map<string,sem_t*>::iterator it=this->mapName.begin(); it!=this->mapName.end(); ++it) {
		sem_close(it->second);
		sem_unlink(it->first.c_str());
	}
}

