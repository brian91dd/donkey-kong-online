#include "cGame.h"

using namespace std;

int main( int argc, char** argv )
{
  if( argc <= 1 )
  {
    printf( "Para poder ejecutar el juego debes ingresar tu nombre!\n" );
    printf( "%s <nombre>\n", argv[0] );
    printf( "Intentalo nuevamente!\n" );
    return 1;
  }

  cGame* game = new cGame( argv[1] );
  game->run();
  free( game );

  return 0;
}
