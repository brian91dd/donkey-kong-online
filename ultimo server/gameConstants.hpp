#ifndef _H_GAME_CONSTANTS
	#define _H_GAME_CONSTANTS
	#include <iostream>

	//Estructuras de mensajes

	struct mensajePlayer {
		int key;
		int direction;
	};

	struct playerMensajeShm {
		char nombreJugador[255];
		int sockFd;
		int puntos;
	};
	//Estructuras de Info
	struct infoPartida {
		int idPlayer1;
		int idPlayer2;
		int idGanador;
		int pidPartida;
		int rescatados1;
		int rescatados2;
		int vidas1;
		int vidas2;
		int partidaEnCurso;
	};

	//DEFINE LAS ACCIONES A REALIZAR CUANDO HAY UNA COMUNICACION
	namespace Protocolo{
		const int TAMBUFF = 500;

		const int PLAYER1_WINS = -1;
		const int PLAYER2_WINS = -2;
		const int SET_PLAYER_ONE = 5;
		const int SET_PLAYER_TWO = 6;
		const int START_GAME = 3;
		const int PLAYER1 = 1;
		const int PLAYER2 = 2;

		const int TOURNAMENT_FINISHED = 4;//no se usan



		const int DK1 = 500;
		const int DK2 = 501;
		const int PRINCESA = 502;
		//PROTOCOLO BARRILES
		const int B1 = 1000;
		const int B2 = 1001;
		const int B3 = 1003;
		//Hasta 1000 barriles
		//const int b1000 = 1999; // Ultimo barril

		//PROTOCOLO FLAMAS
		const int F1 = 2001;
		const int F2 = 2001;
		const int F3 = 2002;
		//Hasta 1000 flamas
		//const int F1000 = 2999;


		//PROTOCOLO DE LUCAS


		const int LEFT = 1;
		const int RIGHT = 2;
		const int DOWN = 3;
		const int UP = 4;
		const int JUMP = 5;
		const int KEYDOWN = 6;
		const int KEYUP = 7;

		//PROTOCOLO DE ESTADOS
		const char LOSE[50] = "Perdio";
		const char PLAYING[50] = "Jugando";
		const char WAITING[50] = "Esperando";
	}


	namespace GameConstants
	{
		const int PLAYER1 = 1;
		const int PLAYER2 = 2;
	}

#endif
