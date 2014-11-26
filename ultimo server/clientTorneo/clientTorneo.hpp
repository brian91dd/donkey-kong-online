#ifndef _H_CLIENT_TORNEO
	#define _H_CLIENT_TORNEO

	#include "../player/player.hpp"
	#include "../helpers/socketStream.hpp"
	#include <iostream>
	#include <fstream>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <vector>

	using namespace std;
	class ClientTorneo: public SocketStream {
		private:
			int portNo;
			string server;
			int playerNum;
			bool endGame;
			bool loss;
			bool tournamentFinished;
		public:
			ClientTorneo(string);
			void setTournamentFinished(bool);
			void setEndGame(bool);
			void setLoss(bool);
			bool isTournamentFinished();
			bool getLoss();
			bool getEndGame();
			int getPlayer();
			void setPlayer(int);
			void startConnection();
	};
#endif