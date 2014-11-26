#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include "./gameConstants.hpp"
#include "./helpers/socketStream.hpp"
#include "./helpers/threadMutexHelper.hpp"
#include "./clientTorneo/clientTorneo.hpp"

/**********GLOBALS***********/
ClientTorneo clientTorneo("parametersClient.txt");
ThreadMutexHelper threadsMutex;
pthread_t threadEventListener,threadReceiver;
//pthread_t sockListenerThread;
/********END GLOBALS*********/


/********PROTOTYPES*********/
void *eventListener(void *);
void *receiver(void *);

/********END PROTOTYPES*********/

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

using namespace std;

int main(int argc, char const *argv[])
{

	char nombreChar[255];
	string nombreStr;
	int comenzar;

	/*
	Opcional: Crear demonio que libere recursos en caso de falla.
	-Se podria preguntar al servidor si hay algun cliente conectado al server en la misma pc
	si lo hay me fijo si hay algun demonio abierto
	*/
	//CREACION DE THREADS MUTEX
	threadsMutex.createMutex("analizeSituation");
	threadsMutex.Lock("analizeSituation");
	//END CREACION DE THREADS MUTEX
	cout << "Ingresar el nombre del jugador: ";
	bzero(nombreChar,sizeof(nombreChar));
	getline(cin,nombreStr);
	strcpy(nombreChar,nombreStr.c_str());
	//Comienzo la conexion con el servidor
	clientTorneo.startConnection();
	//Envio el nombre ingresado por parametro
	clientTorneo.send(nombreChar,strlen(nombreChar));

	//while en juego
	cout << "Esperando a comenzar la partida: " << endl;

	pthread_create(&threadReceiver, NULL, receiver, NULL);
	pthread_create(&threadEventListener, NULL, eventListener, NULL);
	while(true) {
		threadsMutex.Lock("analizeSituation");
		/*
		Por ahora el thread de escucha de eventos se crea al empezar la partida y se destruye al terminarla
		para que no quede bloqueado en un "cin"
		*/
		if(clientTorneo.isTournamentFinished()) {//SI se desconecto el servidor
			break;
		}
		if(!clientTorneo.getLoss()) {
			//pthread_create(&threadEventListener, NULL, eventListener, NULL);

		} else {
			cout << "Como perdiste, termino el torneo para vos" << endl;
			break;
		}
	}
	pthread_join(threadEventListener,NULL);
	//ACA DEBE ESTAR ADENTRO DE UN WHILE MOSTRANDO COMO SE VA SUCEDIENTO LAS PARTIDA
	if(clientTorneo.isTournamentFinished()) {
		cout << "El torneo ha finalizado" << endl;
		cout << "Resultados Finales" << endl;
	} else {
		cout << "El torneo sigue en curso" << endl;
		cout << "Resultados Parciales" << endl;
	}
	pthread_join(threadReceiver,NULL);
	cout << "Terminó el torneo" << endl;
	//END while en juego
	//SE TENDRIA QUE QUEDAR ESCUCHANDO EL RESULTADO DE LAS PARTIDAS RESTANTES
	return 0;
}
//**************************************************
//SIGUIENTE PASO, VER QUE PUEDA SEGUIR ENFRENTANDOSE
//**************************************************

void *receiver(void *) {
	int protocoloPlayer,protocoloRival,posXPlayer,posXRival,posYPlayer,posYRival;
	char cosasPantalla[Protocolo::TAMBUFF],mensaje[Protocolo::TAMBUFF];
	while(1) {
		stringstream mensajeRecibido;
		vector<string> dataMsg;
		bzero(mensaje,Protocolo::TAMBUFF);
		clientTorneo.recv(mensaje,Protocolo::TAMBUFF);
		cout << "Recibido: " << mensaje << endl;

		//mensajeRecibido << mensaje; // Convierto el char a string
		//dataMsg = split(mensajeRecibido.str(), ' ');//Hago un split del char
		/*if(atoi(dataMsg[0].c_str()) == Protocolo::START_GAME) {
			threadsMutex.Unlock("analizeSituation");
		}*/
		/*if(atoi(dataMsg[0].c_str()) == Protocolo::WAIT_TIME) {
			cout << "Segundos faltantes: " << dataMsg[1] << endl;
		}*/
	}
	/*while(1) {
		char mensaje[Protocolo::TAMBUFF];
		bzero(mensaje,Protocolo::TAMBUFF);
		clientTorneo.recv(&mensaje,sizeof(Protocolo::TAMBUFF));
		ssprintf(mensaje,"%d|%d|%d|%d|%d|%d|%s",protocoloPlayer,posXPlayer,posYPlayer,protocoloRival,posXRival,posXPlayer,cosasPantalla);
		if(validacion == Protocolo::PLAYER2_WINS || validacion == Protocolo::PLAYER1_WINS) {
			cout << "Ha ganado el jugador " << validacion * -1 << endl;
			if((validacion * -1) != clientTorneo.getPlayer()) {//PERDEDOR
				clientTorneo.setLoss(true);
				threadsMutex.Unlock("analizeSituation");
				break;
			}

			pthread_cancel(threadEventListener);
		} else if(protocoloPlayer == Protocolo::START_GAME) {
			cout << "Comenzar Juego" << endl;
			threadsMutex.Unlock("analizeSituation");
		} else if(protocoloPlayer == Protocolo::SET_PLAYER_ONE) {
			cout << "Soy el jugador 1" << endl;
			clientTorneo.setPlayer(Protocolo::PLAYER1);
		} else if(protocoloPlayer == Protocolo::SET_PLAYER_TWO) {
			cout << "Soy el jugador 2" << endl;
			clientTorneo.setPlayer(Protocolo::PLAYER2);
		} else if(protocoloPlayer == Protocolo::TOURNAMENT_FINISHED) {
			cout << "El torneo ha finalizado" << endl;
			clientTorneo.setTournamentFinished(true);
			threadsMutex.Unlock("analizeSituation");
			break;
		}
	}*/
	pthread_exit(NULL);
}

void *eventListener(void *) {
	string comando;
	while(1) {
		cout << "Ingresa un comando: ";
		getline( cin, comando );
		cout << "ENVIAR" << comando << endl;
		if(clientTorneo.send(comando.c_str(),Protocolo::TAMBUFF) <= 0) {
			cout << "No se pudo enviar el mensaje" << endl;
		}

	}
	pthread_exit(NULL);
}

