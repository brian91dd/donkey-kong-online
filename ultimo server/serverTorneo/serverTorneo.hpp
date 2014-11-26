#ifndef _H_SERVER_TORNEO
	#define _H_SERVER_TORNEO

	#include "../gameConstants.hpp"
	#include "../player/player.hpp"
	#include "../helpers/socketStream.hpp"
	#include "../helpers/semaphoreHelper.hpp"
	#include <iostream>
	#include <fstream>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include <vector>
	#include <sstream>
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <sys/wait.h>
	#include <unistd.h>

	using namespace std;

	class ServerTorneo: public SocketStream {
		private:
			int portNo;
			int timeMaxAcceptConn;
			int cantRescatadas;
			bool endTorneo;

			SemaphoreHelper semaforosHelper;

			vector<Player> players; //Los que estan en juego (facilita los calculos de quien juega contra quien tener los eliminados aparte)
			vector<Player> playersEliminados; //Los eliminados
			map<int,struct infoPartida> partidasJugadas; //int => el el pid del programa (ver si lo dan de baja hay q actualizar el indice)
		public:
			int shmCrearPartidas,shmComunicationPartida;
			struct playerMensajeShm *playerShm;
			struct infoPartida *partidaShm;

			ServerTorneo(string);
			void savePlayer(Player);
			void createPartidas();
			void playerLose(int);
			bool verificarEndTorneo();
			bool verificarEndPartidas();
			bool acceptPlayer();
			void listenServerPartidas();
			int getPortNo();
			int getTimeMaxAcceptConn();
			Player getPlayer(int);
			void sendStatsMsg();
			void close(int);
	};
#endif
