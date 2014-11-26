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
#include <sstream>
#include <fstream>
#include <vector>
#include "./gameConstants.hpp"
#include "./serverTorneo/serverTorneo.hpp"
#include "./helpers/semaphoreHelper.hpp"
#include "./helpers/fileManager.hpp"
#include "./helpers/threadMutexHelper.hpp"

#define EXIT_SERVER_SUCCESS -1

/******************VARIABLES GLOBALES********************/

//COMIENZO EL SERVER CON ESTA PRIMERA LLAMADA
ServerTorneo serverTorneo("parameters.txt");//Tiene toda la papa del server
SemaphoreHelper semaforos;//Handler de semaforos
ThreadMutexHelper threadsMutex;//Handler de semaforos mutex de threads
vector<Player> players;

int semFile;
key_t keySemFile;
pthread_t sockListenerThread,serverListenerThread,stopAcceptTimerThread;
ofstream fileM;

bool aceptarConexiones = true;
/******************END VARIABLES GLOBALES****************/
//PROTOTIPOS
void *sockListener(void *);
void term(int);
void *listenServersPartidas(void *);
void *stopAcceptTimer(void *);

int main(int argc, char *argv[])
{
	int rc;


	//SEÑALES
	struct sigaction closeSignal,endConnSignal;
	//CAPTURO TERM - CTRL + C - SEGMENTATION FAULT
    memset(&closeSignal, 0, sizeof(struct sigaction));
    closeSignal.sa_handler = term;
    sigaction(SIGTERM, &closeSignal, NULL);
    sigaction(SIGINT, &closeSignal, NULL);
    sigaction(SIGSEGV, &closeSignal, NULL);

    //alarm(serverTorneo.getTimeMaxAcceptConn());
    //END SEÑALES

    /*
    Crear demonio que cierre todos los recursos en caso de falla del servidor
    */

    //THREADS MUTEX
    threadsMutex.createMutex("comenzarPartidas");
    threadsMutex.Lock("comenzarPartidas");//el mutex comenzarPartidas comienza en cero
    //END THREADS MUTEX

    //LANZO EL LISTENER DE SERVIDORES DE PARTIDA
	pthread_create(&serverListenerThread, NULL, listenServersPartidas, NULL);
    //LANZO EL LISTENER DE CONEXIONES
	pthread_create(&sockListenerThread, NULL, sockListener, NULL);//listenServersPartidas
	//LANZO UN THREAD QUE VA A ENVIAR A TODOS LOS CLIENTES CUANDO VA A EMPEZAR EL SERVER
	int secs = serverTorneo.getTimeMaxAcceptConn();
	int *secsPoint = &secs;
	pthread_create(&stopAcceptTimerThread, NULL, stopAcceptTimer, secsPoint);//listenServersPartidas

	/*
	-Crear un thread que escuche los resultados de cada partida
	*/
	//LUEGO DE LANZAR EL LISTENER DE CLIENTES TRABO EL MAIN PARA LUEGO SEGUIR EN LA CREACION DE PARTIDAS
	threadsMutex.Lock("comenzarPartidas");

	//CREACION DE PARTIDAS
	serverTorneo.createPartidas();
	//VERIFICO SI EL TORNEO TERMINO
	while(true) {
		if(serverTorneo.verificarEndPartidas()) {
			if(!serverTorneo.verificarEndTorneo()) {
				cout << "El torneo aún no ha terminado" << endl;
				serverTorneo.createPartidas();
			} else {
				cout << "El torneo ha finalizado" << endl;
				serverTorneo.sendAll(&Protocolo::TOURNAMENT_FINISHED,sizeof(int));
				break;
			}
		} else {
			//No terminaron las partidas
		}
	}

	/*
	Enviar mensaje mostrando quien es el ganador del torneo
	*/

	//LIBERO RECURSOS
	term(-1);
	return 0;
}

void *listenServersPartidas(void *) {
	serverTorneo.listenServerPartidas();
}

void *sockListener(void *threadid)
{
	char buffer[255];
	int clientSock;
	pid_t procesoJuego;
	Player player;

	cout << "Comenzando a escuchar conexioness"<<endl;
	while(aceptarConexiones) {
		serverTorneo.acceptPlayer();
	}
	threadsMutex.Unlock("comenzarPartidas");
	pthread_exit(NULL);
}
void *stopAcceptTimer(void *secsPoint) {
	int secs = *((int *)secsPoint);
	int finish = 0;
	for (int i = secs; i > 0; i--)	{
		stringstream mensajeEnviarStream;
		string mensajeEnviar;
		mensajeEnviarStream << "WAIT_TIME" << " " << i << "|";
		mensajeEnviar = mensajeEnviarStream.str();
		cout << "Quedan segundos Segundos: " << mensajeEnviar << endl;
		serverTorneo.sendAll(mensajeEnviar.c_str(),Protocolo::TAMBUFF);
		sleep(1);
	}
	threadsMutex.Unlock("comenzarPartidas");
	pthread_exit(NULL);
}
/*****CERRADO DE RECURSOS*****/
void term(int signum) {
	cout << "TERMINO: " << signum << endl;
	if(signum == 11) {
		cout << "SEGMENTATION FAULT EN SERVIDOR DE TORNEO" << endl;
	}
	cout << "cerrando socket" << endl;
	if(signum == -1) { //Termino todo correctamente
		serverTorneo.close(0);
	} else {
    	serverTorneo.close(1);
	}
    cout << "cerrando file" << endl;
    fileM.close();
    cout << "cerrando semaforos" << endl;
    semaforos.close();
    //pthread_cancel(sockListenerThread);
    //pthread_cancel(serverListenerThread);
    cout << "Termino de cerrar todo" << endl;

    //PARA QUE NO HAGA EXIT SI TODO TERMINA BIEN
    //if(signum == EXIT_SERVER_SUCCESS) {
    	exit(0);
    //}
}
