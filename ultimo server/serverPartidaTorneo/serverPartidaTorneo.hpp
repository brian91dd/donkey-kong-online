#ifndef _H_SERVER_PARTIDA
	#define _H_SERVER_PARTIDA

	#include "../gameConstants.hpp"
	#include "../helpers/socketStream.hpp"
	#include "../helpers/semaphoreHelper.hpp"
	#include "../player/player.hpp"
	#include <iostream>
	#include <fstream>
	#include <sstream>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include <time.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>





	class ServerPartidaTorneo: public SocketStream {
		private:
			Player player1;
			Player player2;
			int ganador;//Puede ser 1 o 2 dependiendo de que jugador gano o 0 si no gano nadie aun
			int cantRescatar;
			//id Memoria compartida
			int shmIdPlayers;
			int shmComunicationPartida;
			//mensajes de memoria compartida
			struct playerMensajeShm *playersPartida;
			struct infoPartida *partidaShm;
			//END memoria compartida
			pthread_t threadListenPlayer1,threadListenPlayer2;
			SemaphoreHelper semaforosHelper;

		public:
			ServerPartidaTorneo();

			int getGanador();
			void setGanador(int);
			Player getPlayer(const int);
			void sendDataToServer(struct infoPartida);
			void sendEndGameToServer();

			bool sendAllPlayers(const void *,int);
			bool sendPlayer(int, const void *,int);//COMO PRIMER PARAMETRO SE LE PASA EL JUGADOR: ServerPartidaTorneo::PLAYER1 o ::PLAYER2
			bool recvPlayer(int, void *,int);//COMO PRIMER PARAMETRO SE LE PASA EL JUGADOR: ServerPartidaTorneo::PLAYER1 o ::PLAYER2

			void *listenerPlayer(void *);//COMO PRIMER PARAMETRO SE LE PASA EL JUGADOR: ServerPartidaTorneo::PLAYER1 o ::PLAYER2
			void startGame();
			void getPlayers();
			void setRescatar(int);
			int getRescatar();
			void close();
			string charToString(char *);

			//SIMULADOR: Funciones que simulan la logica del juego
			bool validarMovimiento();

			//acumuladores de movimientos
			vector<struct mensajePlayer> movPlayer1;
			vector<struct mensajePlayer> movPlayer2;
	};
#endif