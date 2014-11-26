
#ifndef _H_SOCKET_STREAM
	#define _H_SOCKET_STREAM
	#include <iostream>
	#include <vector>
	#include <fstream>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <errno.h>

	using namespace std;
	class SocketStream {
		protected:
			int sockfd;
			int newSockFd;
			int portNo;
			int serverOrClient;
			string hostName;
			//sockaddr_in Contiene la estructura de una direccion en internet . <netinet/in.h>
			//serv_addr la direccion del server, cli_addr la direccion del cliente
			struct sockaddr_in serv_addr, cli_addr;

			vector<int> clientSockets;
		public:
			SocketStream();
			//Realiza el bindeo para comenzar el server
			void startServer(int);
			void connectToServer(string,int);
			void sendAll(const void *,int);
			void error(const char *);
			virtual void close();
			//No hace rebindeo, lo que hago es checkear que el socketfd este abierto si lo esta devuelve true, sino false y termina
			bool reconnectServer(int);
			bool isConnected();
			bool send(int,const void *,int);
			bool recv(int,void *,int);
			bool send(const void *,int);
			bool recv(void *,int);
			int acceptConn();
			int getSockFd();
	};

#endif