#include "cOil.h"

#include "Misc.h"
#include <string>
#include <cstdlib>
#include <time.h>

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cOil::cOil( int x, int y, char direction, char difficulty ) : x(x), y(y), direction(direction), cant_flames(0)
{
  // Cargo los sprites
  this->oil.add_image( "images/oil_1.png", 600 );
  this->oil.add_image( "images/oil_2.png", 600 );

  // Seteo el ancho y alto (lo tomo de una de las imagenes, pero podria hardcodearlo)
  this->w = this->oil.w;
  this->h = this->oil.h;

  // Segun la dificultad es la cantidad maxima de barriles que lanza
  switch( difficulty )
  {
    case DIF_EASY:
      this->max_flames = 2;
      this->delay_between_flames = 20000;
      break;
    case DIF_NORMAL:
      this->max_flames = 3;
      this->delay_between_flames = 15000;
      break;
    case DIF_HARD:
      this->max_flames = 5;
      this->delay_between_flames = 10000;
      break;
  }

  // Inicializo el seed del rand()
  srand( time( NULL ) );

  // Empiezo a contar el tiempo desde que fue creado para tirar la primer llama
  this->flames_throw_time = SDL_GetTicks() + 500 + ( rand() % 1000 );
}

/******************************************************************************
* Actualiza la animacion e intenta arrojar un llama
*******************************************************************************/
void cOil::update( SDL_Surface* level_mask )
{
  // Intenta lanzar una llama (dependiendo de cuando fue la ultima vez que tiró la ultima)
  if( ( this->flames_throw_time + this->delay_between_flames < SDL_GetTicks() ) && this->cant_flames < this->max_flames )
  {
    this->flames_throw_time = SDL_GetTicks() + ( rand() % 500 );
    cFlame* flame = new cFlame();
    int flame_x = ( this->direction == DIR_RIGHT ) ? this->x + this->w + 2 : this->x - flame->w - 2;
    int flame_y = this->y + this->h - flame->h;
    flame->start( flame_x, flame_y, this->direction );
    this->obj_flames.push_back( flame );
    this->cant_flames++;
  }

  // Actualiza la posicion de las llamas
  list<cFlame*>::iterator p = this->obj_flames.begin();
  while( p != this->obj_flames.end() )
  {
    cFlame* flame = *p;
    flame->update( level_mask );
    ++p;
  }
}

/******************************************************************************
* Renderiza el objeto en pantalla
*******************************************************************************/
void cOil::render( SDL_Surface* screen )
{
  SDL_Rect object_pos;
  object_pos.x = this->x;
  object_pos.y = this->y;
  this->oil.animate();
  SDL_BlitSurface( this->oil.get_image(), 0, screen, &object_pos );

  list<cFlame*>::iterator p = this->obj_flames.begin();
  while( p != this->obj_flames.end() )
  {
    cFlame* flame = *p;
    flame->render( screen );
    p++;
  }
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cOil::~cOil()
{
  list<cFlame*>::iterator p = this->obj_flames.begin();
  while( p != this->obj_flames.end() )
  {
    free( *p );
    p++;
  }
}
