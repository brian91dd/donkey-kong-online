#include "./clientTorneo.hpp"

ClientTorneo::ClientTorneo(string archivo) :SocketStream() {
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
		} else if(aux1 == "SERVER") {
			this->server = aux2;
		} else {
			error = 1;
		}
	}

	if(error==1){
		cerr<<"Error en el formato del archivo de parámetros";
		exit(1);
	}
	this->tournamentFinished = false;
}
void ClientTorneo::startConnection() {
	this->connectToServer(this->server,this->portNo);
}


/*
Getters y setters
*/

bool ClientTorneo::isTournamentFinished() {
	return this->tournamentFinished;
}
void ClientTorneo::setTournamentFinished(bool finished) {
	this->tournamentFinished = finished;
}
void ClientTorneo::setEndGame(bool endGame) {
	this->endGame = endGame;
}
void ClientTorneo::setLoss(bool loss) {
	this->loss = loss;
}
bool ClientTorneo::getLoss() {
	return this->loss;
}
bool ClientTorneo::getEndGame() {
	return this->endGame;
}
void ClientTorneo::setPlayer(int playerNum) {
	this->playerNum = playerNum;
}
int ClientTorneo::getPlayer() {
	return this->playerNum;
}



