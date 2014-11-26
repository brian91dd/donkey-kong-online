#ifndef cTCPSocketH
#define cTCPSocketH

#include <string>
#include <netinet/in.h>

class cTCPSocket
{
  public:
    cTCPSocket();
    /* Client: Connectarse a un server */
    void connect( const std::string& host, unsigned int port );
    /* Server: Escuchar clientes */
    void listen( unsigned int port, unsigned int backlog );
    /* Client/Server: Enviar datos */
    void send( const char* stream, unsigned int size );
    /* Client/Server: Recibir datos */
    int receive( char* stream, unsigned int size );
    /* Server: Aceptar coneccion */
    cTCPSocket* accept();
    /* Client/Server: Deshabilita el send/recv */
    void shutdown();
    /* Client/Server: Cierra el socket */
    void close();
    /* Client/Server: Chequea si esta cerrado */
    bool is_closed();
    /* Client/Server: Cierra el socket, libera recursos de la dll */
    virtual ~cTCPSocket();

  protected:
    int sockDesc;

  private:
    /* Server: Crea un socket con un SD dado */
    cTCPSocket( int sockDesc );
    /* Pasando una direccion y puerto me devuelve la struct necesaria para el connect() */
    sockaddr_in getAddressStruct( const std::string& address, unsigned int port );
    /* Flag que indica si ya fue hecha la etapa de init de sockets en WIN32 */
    static bool initialized;
    /* No permite el constructor de copia ni la asigancion de sockets */
    cTCPSocket( const cTCPSocket& );
    cTCPSocket &operator=( const cTCPSocket& );

};

#endif
