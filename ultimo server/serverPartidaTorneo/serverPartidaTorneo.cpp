#include "./serverPartidaTorneo.hpp"

ServerPartidaTorneo::ServerPartidaTorneo() :SocketStream() {

}

void ServerPartidaTorneo::getPlayers() {

	playerMensajeShm playerShm1,playerShm2;
	this->ganador = 0;
	key_t shmKey = ftok(".",'M');
	if(shmKey == -1) {
		perror("ftok failed: ");
	}
	this->shmIdPlayers = shmget(shmKey,sizeof(struct playerMensajeShm),0);
	if(this->shmIdPlayers <= -1) {
		perror("shmget: shmget failed");
	}
	playersPartida = (struct playerMensajeShm *) shmat(shmIdPlayers,0,0);

	//Me conecto a los semaforos
	semaforosHelper.connectToSemaphore("puedeEnviar");
	semaforosHelper.connectToSemaphore("mutexPlayersEnfrentados");
	semaforosHelper.connectToSemaphore("buffPlayersEnfrentados");

	//Espero por dos jugadores
	semaforosHelper.P("buffPlayersEnfrentados");
	semaforosHelper.P("mutexPlayersEnfrentados");
	cout << this->playersPartida->nombreJugador << endl;
	playerShm1 = *this->playersPartida;
	semaforosHelper.V("mutexPlayersEnfrentados");
	semaforosHelper.V("puedeEnviar"); //PARA QUE EL SERVER PUEDA CREAR LA SIGUIENTE PARTIDA

	semaforosHelper.P("buffPlayersEnfrentados");
	semaforosHelper.P("mutexPlayersEnfrentados");
	cout << this->playersPartida->nombreJugador << endl;
	playerShm2 = *this->playersPartida;
	semaforosHelper.V("mutexPlayersEnfrentados");
	semaforosHelper.V("puedeEnviar"); //PARA QUE EL SERVER PUEDA CREAR LA SIGUIENTE PARTIDA

	this->player1.setName(this->charToString(playerShm1.nombreJugador));
	this->player2.setName(this->charToString(playerShm2.nombreJugador));
	this->player1.setSockFd(playerShm1.sockFd);
	this->player2.setSockFd(playerShm2.sockFd);

	shmdt(this->playersPartida);
	cout << "Player 1: " << this->player1.getName() << "Con socket: " << this->player1.getSockFd() << endl;
	cout << "Player 2: " << this->player2.getName() << "Con socket: " << this->player2.getSockFd() << endl;

	//ME CONECTO A LA MEMORIA COMPARTIDA PARA ENVIAR LA INFO DE LA PARTIDA AL SERVIDOR
	shmKey = ftok(".",'C');
	if(shmKey == -1) {
		perror("shmkey: shmkey failed");
	}
	this->shmComunicationPartida = shmget(shmKey,sizeof(struct infoPartida),0);
	if(this->shmComunicationPartida == -1) {
		perror("shmget: shmget failed");
	}
	this->partidaShm = (struct infoPartida *) shmat(this->shmComunicationPartida,0,0);
	//Me conecto a los semaforos
	semaforosHelper.connectToSemaphore("mutexInfoPartida");
	semaforosHelper.connectToSemaphore("buffInfoPartida");
	semaforosHelper.connectToSemaphore("lugarBuffInfoPartida");

}

void ServerPartidaTorneo::sendDataToServer(struct infoPartida infoJugadorEnviar) {
	semaforosHelper.P("lugarBuffInfoPartida");
	semaforosHelper.P("mutexInfoPartida");
	*(this->partidaShm) = infoJugadorEnviar;
	semaforosHelper.V("mutexInfoPartida");
	semaforosHelper.V("buffInfoPartida");
}

void ServerPartidaTorneo::startGame() {
	/*stringstream mensajeEnviarStr;
	string converterToChar;
	mensajeEnviarStr << Protocolo::NEW_GAME << " " << this->player1.getName() << ";" << this->player2.getName() << ";" << Protocolo::VIDAS_INICIALES << ";" << Protocolo::X_DK_1 << ";" << Protocolo::Y_DK_1 << ";" << Protocolo::DIRECTION_DK_1 << ";" << Protocolo::X_DK_2 << ";" << Protocolo::Y_DK_2 << ";" << Protocolo::DIRECTION_DK_2 << ";" << Protocolo::X_OIL_1 << ";" << Protocolo::Y_OIL_1 << ";" << Protocolo::DIRECTION_OIL_1 << ";" << Protocolo::X_OIL_2 << ";" << Protocolo::Y_OIL_2 << ";" << Protocolo::DIRECTION_OIL_2 << ";" << Protocolo::X_PAULINE << ";" << Protocolo::Y_PAULINE;
	converterToChar = mensajeEnviarStr.str();
	cout << converterToChar << endl;
	sendAllPlayers(converterToChar.c_str(),Protocolo::TAMBUFF);*/
}


void ServerPartidaTorneo::sendEndGameToServer() {
	/*key_t shmKey = ftok(".",'M');
	if(shmKey == -1) {
		perror("shmkey: shmkey failed");
	}
	shmCrearPartidas = shmget(shmKey,sizeof(struct playerMensajeShm),IPC_CREAT|IPC_EXCL|0666);
	if(shmCrearPartidas == -1) {
		perror("shmget: shmget failed");
	}
	playerShm = (struct playerMensajeShm *) shmat(this->shmCrearPartidas,0,0);*/
}

//SIMULADO: Funcion que simula la validacion del movimiento
bool ServerPartidaTorneo::validarMovimiento() {
	//RANDOMS SOLO PARA PROBAR
	int numRand;
	srand (time(NULL));
	numRand = rand() % 2 + 1;
	return (numRand == 1 ? 1 : 0);
}



bool ServerPartidaTorneo::sendAllPlayers(const void *msg,int size) {
	//cout << "SOCKETS PLAYERS 1 y 2 : " << this->player1.getSockFd() << " y " <<  this->player2.getSockFd() << endl;
	if(!this->send(this->player1.getSockFd(),(void *) msg,size)) {
		cout << "No se envio 1" << endl;
		//PUEDE SER DESCONEXION DE JUGADOR
		return false;
	}
	if(!this->send(this->player2.getSockFd(),(void *) msg,size)) {
		cout << "No se envio 2" << endl;
		//PUEDE SER DESCONEXION DE JUGADOR
		return false;
	}
	//cout << "Se envio a los dos" << endl;
	return true;
}
bool ServerPartidaTorneo::sendPlayer(int playerNum, const void *msg,int size) {
	Player playerSelected((playerNum == GameConstants::PLAYER1 ? this->player1 : this->player2));
	if(!this->send(playerSelected.getSockFd(),(void *) msg,size)) {
		//PUEDE SER DESCONEXION DE JUGADOR
		return false;
	}
	return true;
}

bool ServerPartidaTorneo::recvPlayer(int playerNum, void *msg,int size) {
	Player playerSelected((playerNum == GameConstants::PLAYER1 ? this->player1 : this->player2));

	if(!this->recv(playerSelected.getSockFd(),(void *) msg,size)) {
		perror("Error al hacer recv: ");
		//PUEDE SER DESCONEXION DE JUGADOR
		return false;
	}
	return true;
}

string ServerPartidaTorneo::charToString(char *convert) {
	stringstream ss;
	string s;
	ss << convert;
	ss >> s;
	return s;
}

int ServerPartidaTorneo::getGanador() {
	return this->ganador;
}
void ServerPartidaTorneo::setGanador(int ganador) {
	this->ganador = ganador;
}
Player ServerPartidaTorneo::getPlayer(int numPlayer) {
	return (numPlayer  == GameConstants::PLAYER1 ? this->player1 : this->player2);
}

void ServerPartidaTorneo::close() {
	for (vector<int>::iterator it = this->clientSockets.begin() ; it != this->clientSockets.end(); ++it) {
		//cout << *it <<endl;
		::close(*it);
	}
	//cout << this->sockfd<<endl;
	::close(this->sockfd);
	this->semaforosHelper.close();
	shmdt(this->playersPartida);
	shmdt(this->partidaShm);
}

/*GETTERS Y SETTERS*/
void ServerPartidaTorneo::setRescatar(int cant) {
	this->cantRescatar = cant;
}
int ServerPartidaTorneo::getRescatar() {
	return this->cantRescatar;
}
