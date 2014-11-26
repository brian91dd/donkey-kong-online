#include "socketStream.hpp"
/*
** CON HOSTNAME SE CONECTA
** SIN HOSTNAME CREA UN SERVER
*/
SocketStream::SocketStream() {

}
void SocketStream::startServer(int portNo) {
	int iSetOption;
	this->portNo = portNo;
	//Creao el socket
	this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//Para poder volver a usar el puerto sin tener que esperar un tiempo
	//En la web dice que por default hay un tiempo para volver a usar un puerto
	//http://stackoverflow.com/questions/548879/releasing-bound-ports-on-process-exit/548912#548912
	setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,sizeof(iSetOption));

	bzero((char *) &this->serv_addr, sizeof(this->serv_addr));
	this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY setea la direccion de ip de la PC en la que esta corriendo
	this->serv_addr.sin_port = htons(this->portNo); //seteo el puerto en el que corre

	/* BINDEO el socket al puerto y direccion*/
	if (bind(this->sockfd, (struct sockaddr *) &this->serv_addr, sizeof(this->serv_addr)) < 0) {
		error("ERROR on binding");
	}

	//Permite que el socket comienze a escuchar conexiones, la mayoria de los sistemas permite que escuche hasta 5 conexiones
	listen(this->sockfd,5);
}

void SocketStream::connectToServer(string hostName,int portNo) {
	struct hostent *server;
	this->portNo = portNo;
	this->hostName = hostName;

	this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(this->hostName.c_str());
	bzero((char *) &this->serv_addr, sizeof(this->serv_addr));
	this->serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&this->serv_addr.sin_addr.s_addr,
         server->h_length);
    this->serv_addr.sin_port = htons(this->portNo);
    if (connect(this->sockfd,(struct sockaddr *) &this->serv_addr,sizeof(this->serv_addr)) < 0)
        error("ERROR connecting");
}

bool SocketStream::reconnectServer(int sockfd) {
	this->sockfd = sockfd;
	if(this->isConnected()) {
		return true;
	} else {
		return false;
	}
}

bool SocketStream::isConnected() {
	int error = 0;
	socklen_t len = sizeof (error);
	int retval = getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error, &len );
	if(retval == 0) {
		return true;
	} else {
		return false;
	}
}

int SocketStream::acceptConn() {
	int newsockfd;
	socklen_t clilen;
	clilen = sizeof(cli_addr);
	newsockfd = accept(this->sockfd, (struct sockaddr *) &cli_addr, &clilen);
	this->clientSockets.push_back (newsockfd);
	cout << "conectado con: " << newsockfd<<endl;
	//cout << "Error" << errno << " " << EINTR << endl;
	return newsockfd;
}

bool SocketStream::send(int socket,const void *msg, int size) {
	int n;
	//:: para que tome del scope global y no de la clase
	n = ::send(socket,(void *)msg,size,0);
	if (n < 0) error("ERROR writing to socket");
	return n;
}

void SocketStream::sendAll(const void *msg, int size) {
	int n;
	for (vector<int>::iterator it = this->clientSockets.begin() ; it != this->clientSockets.end(); ++it) {
		n = ::send(*it,msg,size,0);
		if (n < 0) error("ERROR writing to socket");
	}
}

bool SocketStream::recv(int socket,void *msg, int size) {
	int n;
	n = ::recv(socket,msg,size,0);
	if (n < 0) {
		return false;
	}
	return true;
}



bool SocketStream::send(const void *msg, int size) {
	int n;
	//:: para que tome del scope global y no de la clase
	n = ::send(this->sockfd,(void *)msg,size,0);
	if (n < 0) {
		return false;
	}
	return true;
}

bool SocketStream::recv(void *msg, int size) {
	int n;
	n = ::recv(this->sockfd,msg,size,0);
	if (n < 0) {
		return false;
	}
	return true;
}

void SocketStream::error(const char *msg) {
    perror(msg);
    exit(1);
}

void SocketStream::close() {
	for (vector<int>::iterator it = this->clientSockets.begin() ; it != this->clientSockets.end(); ++it) {
		//cout << *it <<endl;
		::close(*it);
	}
	//cout << this->sockfd<<endl;
	::close(this->sockfd);
}


int SocketStream::getSockFd() {
	return this->sockfd;
}