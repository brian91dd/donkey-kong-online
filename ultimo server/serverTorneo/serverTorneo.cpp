#include "serverTorneo.hpp"

/**/

ServerTorneo::ServerTorneo(string archivo) :SocketStream() {
	ifstream archivo_parametros(archivo.c_str());
	string aux1,aux2;
	int error=0, portNo = 6000, timeMax = 20;
	if(!archivo_parametros){
		cerr<<"Error al abrir el archivo";
		exit(1);
	}

	while(!archivo_parametros.eof()) {
		archivo_parametros>>aux1>>aux2;
		if(aux1 == "PUERTO") {
			this->portNo = atoi(aux2.c_str());
		} else if(aux1 == "TIEMPOMAX") {
			this->timeMaxAcceptConn = atoi(aux2.c_str());
		} else if(aux1 == "CANTRESCATADAS") {
			this->cantRescatadas = atoi(aux2.c_str());
		} else {
			error = 1;
		}
	}

	if(error==1){
		cerr<<"Error en el formato del archivo de parámetros";
		exit(1);
	}
	this->startServer(this->portNo);
	this->endTorneo = false;

	//Semaforos para enviar players a servidor
	this->semaforosHelper.createSemaphore("puedeEnviar",1);
	this->semaforosHelper.createSemaphore("mutexPlayersEnfrentados",1);
	this->semaforosHelper.createSemaphore("buffPlayersEnfrentados",0);
	//Semaforos para obtener informacion de los servidores de partida
	this->semaforosHelper.createSemaphore("lugarBuffInfoPartida",1);
	this->semaforosHelper.createSemaphore("buffInfoPartida",0);
	this->semaforosHelper.createSemaphore("mutexInfoPartida",1);
	//Semaforo que despierta el mainthread para que verifique si ya termino el torneo
	this->semaforosHelper.createSemaphore("verifEndTournament",0);

	//CREACION MEMORIA COMPARTIDA COMPARTIDA PARA ENVIAR JUGADORES
	key_t shmKey = ftok(".",'M');
	if(shmKey == -1) {
		perror("shmkey: shmkey failed este");
	}
	this->shmCrearPartidas = shmget(shmKey,sizeof(struct playerMensajeShm),IPC_CREAT|IPC_EXCL|0666);
	if(this->shmCrearPartidas == -1) {
		perror("shmget: shmget failed este");
	}

	this->playerShm = (struct playerMensajeShm *) shmat(this->shmCrearPartidas,0,0);
	//END CREACION MEMORIA COMPARTIDA
}

bool ServerTorneo::acceptPlayer() {
	int clientSock;
	char buffer[255];
	Player player;

	clientSock = this->acceptConn();
	bzero(buffer,sizeof(buffer));
	if(clientSock == -1 || !this->recv(clientSock,buffer,sizeof(buffer))) {
		cout << "Salgo" <<endl;
		return false;
	}

	string buffString(buffer);
	buffString = buffString.substr(0,buffString.size()-1);

	cout << "Nombre del jugador: " << buffString << endl;
	player.setName(buffString);
	player.setSockFd(clientSock);
	this->players.push_back(player);
	return true;
}

void ServerTorneo::createPartidas() {
	int pidServerPartida;
	struct infoPartida infoCreatedPartida;
	//struct playersEnfrentados playersEnfrentamiento;

	//buffer2PlayersEnfrentados->player2.setName("julieta");
	//Elimino uno si es impar
	if((this->players.size() % 2) != 0) {
		Player playerOut(this->players.back());
		this->players.pop_back();

		stringstream mensajeEnviarStream;
		string mensajeEnviar;
		mensajeEnviarStream << "DISCONNECT|";
		mensajeEnviar = mensajeEnviarStream.str();
		cout << "Terminando partida para jugador: " << mensajeEnviar << endl;
		this->send(playerOut.getSockFd(),mensajeEnviar.c_str(),Protocolo::TAMBUFF);
		//SACAR AL CHABONCITO
	}

	for (int primero = 0; primero < floor(this->players.size() / 2); primero++) {
		int ultimo = (this->players.size() - primero - 1);
		cout << this->players.at(primero).getName() << " VS " << this->players.at(ultimo).getName() <<endl;

		//CONVIERTO EL SOCKETFD DEL SERVER A STR PARA DESPUES PASARLO A CHAR Y PASARLO POR PARAMETRO
		stringstream sockFdStream;
		sockFdStream << this->getSockFd();
		string sockFdString = sockFdStream.str();

		stringstream cantRescatadasStream;
		cantRescatadasStream << this->cantRescatadas;
		string cantRescatadasString = cantRescatadasStream.str();
		cout << "Comienzo partidas" << endl;
		if((pidServerPartida = fork()) == 0) {
			const char *av[] = { "./serverPartida",sockFdString.c_str(),cantRescatadasString.c_str(),  NULL };
			execvp(av[0],(char * const *) av);
			_Exit(0);
		} else {
			waitpid( pidServerPartida, NULL, WNOHANG );
			infoCreatedPartida.idPlayer1 = this->players.at(primero).getSockFd();
			infoCreatedPartida.idPlayer2 = this->players.at(ultimo).getSockFd();
			infoCreatedPartida.rescatados1 = 0;
			infoCreatedPartida.rescatados2 = 0;
			infoCreatedPartida.partidaEnCurso = 1;

			this->partidasJugadas[pidServerPartida] = infoCreatedPartida;

			//Envio Jugador 1
			this->semaforosHelper.P("puedeEnviar");
			this->semaforosHelper.P("mutexPlayersEnfrentados");
			strcpy(this->playerShm->nombreJugador,this->players.at(primero).getName().c_str());
			this->playerShm->sockFd = this->players.at(primero).getSockFd();
			this->semaforosHelper.V("mutexPlayersEnfrentados");
			this->semaforosHelper.V("buffPlayersEnfrentados");

			//Envio Jugador 2
			this->semaforosHelper.P("puedeEnviar");
			this->semaforosHelper.P("mutexPlayersEnfrentados");
			strcpy(this->playerShm->nombreJugador,this->players.at(ultimo).getName().c_str());
			this->playerShm->sockFd = this->players.at(ultimo).getSockFd();
			this->semaforosHelper.V("mutexPlayersEnfrentados");
			this->semaforosHelper.V("buffPlayersEnfrentados");
			//Enviar por memoria compartida los dos jugadores pidServerPartida
		}
	}
	//CIERRO LA MEMORIA COMPARTIDA

}
void ServerTorneo::listenServerPartidas() {
	key_t shmKey = ftok(".",'C');
	bool huboGanador = false;
	int perdedor = 0;
	Player p1,p2;
	if(shmKey == -1) {
		perror("shmkey: shmkey failed aca");
	}
	this->shmComunicationPartida = shmget(shmKey,sizeof(struct infoPartida),IPC_CREAT|IPC_EXCL|0666);
	if(this->shmComunicationPartida == -1) {
		perror("shmget: shmget failed aca");
	}
	this->partidaShm = (struct infoPartida *) shmat(this->shmComunicationPartida,0,0);


	while(!this->endTorneo) {
		huboGanador = false;
		perdedor = 0;
		this->semaforosHelper.P("buffInfoPartida");
		this->semaforosHelper.P("mutexInfoPartida");
		//cout << "La partida " << this->partidaShm->pidPartida << " ha enviado la siguiente informacion" << endl;
		if(this->partidaShm->idGanador) {
			//cout << "Hubo un ganador con el id" << this->partidaShm->idGanador << endl;
			this->partidasJugadas[this->partidaShm->pidPartida].partidaEnCurso = 0;
			huboGanador = true;
			perdedor = (this->partidaShm->idGanador == this->partidaShm->idPlayer1 ? this->partidaShm->idPlayer2 : this->partidaShm->idPlayer1); //DEVUELVO EL PERDEDOR
			getPlayer(this->partidaShm->idGanador).setEstado(Protocolo::WAITING);
			getPlayer(perdedor).setEstado(Protocolo::LOSE);
		}
		this->partidasJugadas[this->partidaShm->pidPartida].rescatados1 = this->partidaShm->rescatados1;
		this->partidasJugadas[this->partidaShm->pidPartida].rescatados2 = this->partidaShm->rescatados2;
		this->partidasJugadas[this->partidaShm->pidPartida].vidas1 = this->partidaShm->vidas1;
		this->partidasJugadas[this->partidaShm->pidPartida].vidas2 = this->partidaShm->vidas2;
		getPlayer(this->partidaShm->idPlayer1).setVidas(this->partidaShm->vidas1);
		getPlayer(this->partidaShm->idPlayer1).setPaulines(this->partidaShm->rescatados1);
		getPlayer(this->partidaShm->idPlayer2).setVidas(this->partidaShm->vidas2);
		getPlayer(this->partidaShm->idPlayer2).setPaulines(this->partidaShm->rescatados2);
		//cout << "Rescatadas Jugador 1: " << this->partidasJugadas[this->partidaShm->pidPartida].rescatados1 << endl << " Con vidas: " << this->partidasJugadas[this->partidaShm->pidPartida].vidas1;
		//cout << "Rescatadas Jugador 2: " << this->partidasJugadas[this->partidaShm->pidPartida].rescatados2 << endl << " Con vidas: " << this->partidasJugadas[this->partidaShm->pidPartida].vidas2;
		//Guardar la info de la partida
		sendStatsMsg();
		this->semaforosHelper.V("mutexInfoPartida");
		this->semaforosHelper.V("lugarBuffInfoPartida");
		if(huboGanador) {
			playerLose(perdedor);
			this->semaforosHelper.V("verifEndTournament");
		}
	}
}

void ServerTorneo::sendStatsMsg() {
	//iterar sobre jugador para enviar mensajes

	stringstream mensajeEnviar;
	for (vector<Player>::iterator it = this->players.begin() ; it != this->players.end(); ++it) {
		mensajeEnviar << "STATS ";
		mensajeEnviar << it->getName() << ",";
		mensajeEnviar << it->getVidas() << ",";
		mensajeEnviar << it->getPaulines() << ",";
		mensajeEnviar << "1;";
	}
	mensajeEnviar << "|";

	cout << mensajeEnviar.str() << endl;
	//Mensaje: STATS NOMBRE,ESTADO,VIDAS,PAULINES,POSICION;NOMBRE,ESTADO,VIDAS,PAULINES,POSICION;...
	//Ejemplo: STATS Lucas,Descalificado,0,1,3ero;Marcos,Ganador (Esperando),2,3,-;Brian,Jugando,3,1,-;Pedro,Jugando,2,2,-
}
Player ServerTorneo::getPlayer(int sockfd) {
	for (vector<Player>::iterator it = this->players.begin() ; it != this->players.end(); ++it) {
		if(it->getSockFd() == sockfd) {
			return *it;
		}
	}
}

void ServerTorneo::playerLose(int playerId) {
	for (int i = 0 ; i < this->players.size(); i++) {
		if(this->players.at(i).getSockFd() == playerId) {

			cout << "Eliminando a: " << this->players.at(i).getSockFd() << endl;
			this->playersEliminados.push_back(this->players.at(i));
			this->players.erase(this->players.begin() + i);
			break;
		}
	}
}

bool ServerTorneo::verificarEndPartidas() {
	this->semaforosHelper.P("verifEndTournament");
	for (int i = 0; i < this->partidasJugadas.size(); ++i) {
		if(this->partidasJugadas[i].partidaEnCurso != 0) {
			return false;
		}
	}
	return true;
}
bool ServerTorneo::verificarEndTorneo() {
	if(this->players.size() == 1) {
		return true;
	} else {
		if(this->players.size() == 0) {
			cout << "Hubo algun problema en la creacion de players, la cantidad es cero" << endl;
		}
		return false;
	}
}

void ServerTorneo::savePlayer(Player player) {
	this->players.push_back(player);
}


/******* GETTERS Y SETTERS *******/
int ServerTorneo::getPortNo() {
	return this->portNo;
}
int ServerTorneo::getTimeMaxAcceptConn() {
	return this->timeMaxAcceptConn;
}

void ServerTorneo::close(int num) {
	if(num != 0) {
		string msg = "TOURNAMENT_DOWN|";
		sendAll(msg.c_str(),Protocolo::TAMBUFF);
	}
	for (vector<int>::iterator it = this->clientSockets.begin() ; it != this->clientSockets.end(); ++it) {
		::close(*it);
	}
	::close(this->sockfd);
	this->semaforosHelper.close();
	shmdt(this->playerShm);					// Desvinculacion SM
	shmctl(this->shmCrearPartidas,IPC_RMID,NULL);	// Destruccion SM
	shmdt(this->partidaShm);					// Desvinculacion SM
	shmctl(this->shmComunicationPartida,IPC_RMID,NULL);	// Destruccion SM
}
