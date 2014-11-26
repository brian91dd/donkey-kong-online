/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "./gameConstants.hpp"
#include "./helpers/socketStream.hpp"
#include "./helpers/semaphoreHelper.hpp"
#include "./helpers/threadMutexHelper.hpp"
#include "./serverPartidaTorneo/serverPartidaTorneo.hpp"
#include <fstream>

#include "./Clases/cGame.h"

SemaphoreHelper semaforos;
ServerPartidaTorneo serverPartida;
pthread_t threadListenPlayer1,threadListenPlayer2,threadValidator;
ThreadMutexHelper threadsMutex;

struct playerMensajeShm *playersPartida;

int shmIdPlayers;
cGame* game;

void term(int);
void *listenerPlayerActions(void *);
void *validateMovi(void *);

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

int main(int argc, char *argv[])
{

	int comenzar = 1;
	int rc;
	cout << "COMIENZO SERVER PARTIDA" << endl;

	//SEÑALES
	struct sigaction closeSignal;
    memset(&closeSignal, 0, sizeof(struct sigaction));
    closeSignal.sa_handler = term;
    //CAPTURO TERM - CTRL + C - SEGMENTATION FAULT
    sigaction(SIGTERM, &closeSignal, NULL);
    sigaction(SIGINT, &closeSignal, NULL);
    sigaction(SIGSEGV, &closeSignal, NULL);
    //END SEÑALES

    //INICIALIZO EL OBJETO CON EL SOCKET, NO VUELVE A BINDEAR
    serverPartida.reconnectServer(atoi(argv[1]));
    //OBTENGO LA CANTIDAD DE PRINCESAS QUE SE DEBEN RESCATAR
    serverPartida.setRescatar(atoi(argv[2]));
    //RECIBO LOS JUGADORES
    cout << "Recibiendo Jugadores" << endl;
	serverPartida.getPlayers();
	cout << "Creando threads" << endl;
	//CREO LOS THREADS
	threadsMutex.createMutex("validateMovement");//ESCUCHA DE JUGADORES
	threadsMutex.createMutex("endGame");
	threadsMutex.createMutex("mutexMovimiento1");
	threadsMutex.createMutex("mutexMovimiento2");
	semaforos.createSemaphore("cantMovimientos1",0);
	semaforos.createSemaphore("cantMovimientos2",0);
	cout << "end creacion mutex" << endl;
	//threadsMutex.Lock("endGame");//Para inicializarlo en cero
	int *player1,*player2;
	int p1,p2;

	p1 = Protocolo::PLAYER1;
	p2 = Protocolo::PLAYER2;

	player1 = &p1;
	player2 = &p2;

	pthread_create(&threadListenPlayer1, NULL, listenerPlayerActions, player1);
	pthread_create(&threadListenPlayer2, NULL, listenerPlayerActions, player2);
	pthread_create(&threadValidator, NULL, validateMovi, NULL);
	cout << "Comenzando Juego" << endl;

	//COMIENZO EL JUEGO
	serverPartida.startGame();
	pthread_join(threadListenPlayer1,NULL);
	pthread_join(threadListenPlayer2,NULL);
	pthread_join(threadValidator,NULL);
	//threadsMutex.Lock("endGame");

	//ENVIO DATOS AL SERVER ACERCA DE LA PARTIDA
	term(1);
	return 0;
}
void *listenerPlayerActions(void *playerNumPoint) {
	int validacion,rescatoPrincesa,msgPlayer;
	int playerNum = *((int *)playerNumPoint);

	struct mensajePlayer mensajePlayerStructured;
	char mensajeClient[Protocolo::TAMBUFF];
	vector<string> dataMsg;
	stringstream mensajeRecibido;
	//FILTRO EL JUGADOR QUE LANZO
	Player playerSelected((playerNum == Protocolo::PLAYER1 ? serverPartida.getPlayer(Protocolo::PLAYER1) : serverPartida.getPlayer(Protocolo::PLAYER2)));

	//Enviar al server

	while(1) {
		mensajeRecibido.str("");
		memset(mensajeClient,0,Protocolo::TAMBUFF);
		serverPartida.recvPlayer(playerNum,&mensajeClient,Protocolo::TAMBUFF);
		mensajeRecibido << mensajeClient;

		cout << "mensajeClient" << mensajeClient <<endl;

		cout << "MensajeRecibido" << mensajeRecibido.str() << endl;

		dataMsg = split(mensajeRecibido.str(), ' ');
		mensajePlayerStructured.key = atoi(dataMsg[0].c_str());

		dataMsg[1] = dataMsg[1].substr(0,dataMsg[1].size()-1);

		cout << "Teclas recibidas: " << dataMsg[0] << " " << dataMsg[1] <<endl;

		mensajePlayerStructured.direction = atoi(dataMsg[1].c_str());

		threadsMutex.Lock("validateMovement");
		if(serverPartida.getGanador() != 0) {
			//Por si no se llego a cancelar el thread
			pthread_exit(NULL);
		}

		if(playerNum == Protocolo::PLAYER1) {
			if(mensajePlayerStructured.direction == Protocolo::LEFT) {
				  if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key1["LEFT"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key1["LEFT"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::RIGHT) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key1["RIGHT"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key1["RIGHT"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::DOWN) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key1["DOWN"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key1["DOWN"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::UP) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key1["UP"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key1["UP"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::JUMP) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key1["JUMP"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key1["JUMP"] = false;
				}
			}
		} else {

			if(mensajePlayerStructured.direction == Protocolo::LEFT) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key2["LEFT"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {

					game->key2["LEFT"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::RIGHT) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key2["RIGHT"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {


					game->key2["RIGHT"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::DOWN) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key2["DOWN"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key2["DOWN"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::UP) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key2["UP"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key2["UP"] = false;
				}
			} else if(mensajePlayerStructured.direction == Protocolo::JUMP) {
				if(mensajePlayerStructured.key == Protocolo::KEYDOWN) {
					game->key2["JUMP"] = true;
				} else if(mensajePlayerStructured.key == Protocolo::KEYUP) {
					game->key2["JUMP"] = false;
				}
			}


		}
		threadsMutex.Unlock("validateMovement");
	}
}

void *validateMovi(void *) {
	struct infoPartida infoPartidaEnviar;
	string mensajeEnviarPlayer;
	struct mensajePlayer movPlayer;


	game = new cGame(serverPartida.getPlayer(Protocolo::PLAYER1).getName(),serverPartida.getPlayer(Protocolo::PLAYER2).getName());

	game->new_game( DIF_EASY );
	unsigned int interval = 1000000 / cGame::FPS;

	mensajeEnviarPlayer = game->send_new_game();
	serverPartida.sendAllPlayers(mensajeEnviarPlayer.c_str(),Protocolo::TAMBUFF);

	infoPartidaEnviar.idPlayer1 = serverPartida.getPlayer(Protocolo::PLAYER1).getSockFd();
	infoPartidaEnviar.idPlayer2 = serverPartida.getPlayer(Protocolo::PLAYER2).getSockFd();
	infoPartidaEnviar.idGanador = 0;
	infoPartidaEnviar.pidPartida = getpid();
	infoPartidaEnviar.rescatados1 = 0;
	infoPartidaEnviar.rescatados2 = 0;
	infoPartidaEnviar.vidas1 = game->p1->lives;
	infoPartidaEnviar.vidas2 = game->p2->lives;
	infoPartidaEnviar.partidaEnCurso = 1;
	while(serverPartida.getGanador() == 0) {
		mensajeEnviarPlayer = game->send_update_map();
		serverPartida.sendAllPlayers(mensajeEnviarPlayer.c_str(),Protocolo::TAMBUFF);
		if(game->p1->lives == 2) {//serverPartida.getRescatar()
			infoPartidaEnviar.idGanador = infoPartidaEnviar.idPlayer1;
			cout << "Ha ganado un jugador: " << game->p1->name << endl;
			mensajeEnviarPlayer = "YOU_WIN|";
			serverPartida.sendPlayer(Protocolo::PLAYER1,mensajeEnviarPlayer.c_str(),Protocolo::TAMBUFF);
			mensajeEnviarPlayer = "YOU_LOSE|";
			serverPartida.sendPlayer(Protocolo::PLAYER2,mensajeEnviarPlayer.c_str(),Protocolo::TAMBUFF);
			serverPartida.sendDataToServer(infoPartidaEnviar);
			pthread_cancel(threadListenPlayer1);
			pthread_cancel(threadListenPlayer2);
			pthread_exit(NULL);
		}
		if(game->p2->lives == 2) {//serverPartida.getRescatar()
			infoPartidaEnviar.idGanador = infoPartidaEnviar.idPlayer2;
			cout << "Ha ganado un jugador: " << game->p2->name << endl;
			mensajeEnviarPlayer = "YOU_LOSE|";
			serverPartida.sendPlayer(Protocolo::PLAYER1,mensajeEnviarPlayer.c_str(),Protocolo::TAMBUFF);
			mensajeEnviarPlayer = "YOU_WIN|";
			serverPartida.sendPlayer(Protocolo::PLAYER2,mensajeEnviarPlayer.c_str(),Protocolo::TAMBUFF);
			serverPartida.sendDataToServer(infoPartidaEnviar);
			pthread_cancel(threadListenPlayer1);
			pthread_cancel(threadListenPlayer2);
			pthread_exit(NULL);
		}


		game->p1->update( game->key1, game->level_mask );
		game->p2->update( game->key2, game->level_mask );
		game->update_playing();
		//game->render_playing();
		SDL_Flip( game->screen );

		//Envio datos al servidor
		infoPartidaEnviar.vidas1 = game->p1->lives;
		infoPartidaEnviar.vidas2 = game->p2->lives;
		infoPartidaEnviar.rescatados1 = game->p1->wins;
		infoPartidaEnviar.rescatados2 = game->p2->wins;
		serverPartida.sendDataToServer(infoPartidaEnviar);
		usleep( interval );
	}
}

/*****CERRADO DE RECURSOS*****/
void term(int signum) {
	if(signum == 11) {
		cout << "SEGMENTATION FAULT EN SERVER PARTIDA" << endl;
	}
	cout << "cerrando socket" << endl;

    cout << "cerrando semaforos" << endl;
    semaforos.close();
    threadsMutex.close();
    //pthread_cancel(sockListenerThread);
    /*
    Hay que sacarlo, el que marca para borrar es el server, el detach se hace en serverPartidaTorneo
    shmdt(playersPartida);					// Desvinculacion SM
	shmctl(shmIdPlayers,IPC_RMID,NULL);	// Destruccion SM

	shmdt(partidaShm);					// Desvinculacion SM
	shmctl(shmComunicationPartida,IPC_RMID,NULL);	// Destruccion SM*/
    cout << "Termino de cerrar todo EN SERVER DE PARTIDA" << endl;
    exit(0);
}
