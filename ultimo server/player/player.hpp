#ifndef _H_JUGADOR
	#include "../gameConstants.hpp"
	#include <stdio.h>
	#include <string.h>
	#include <iostream>
	using namespace std;
	class Player {
		private:
			string name;
			int sockFd;
			int vidas;
			int paulines;
			int paulinesTotales;
			int vidasPerdidasTotales;
			string estado;
		public:
			Player();
			Player(string);
			Player(Player*);
			void setName(string);
			string getName();
			string getEstado();
			void setSockFd(int);
			int getSockFd();
			int getVidas();
			int getPaulines();
			int getVidasTotales();
			int getPaulinesTotales();//El seter pasa po setPaulines
			bool isPlaying();
			bool isEliminated();
			void setVidas(int);
			void setPaulines(int);
			void setEstado(string);

			bool eliminado;
			//Player& operator=(Player);
	};
#endif