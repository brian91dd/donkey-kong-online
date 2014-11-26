#include "cDonkeyKong.h"

#include "Misc.h"
#include <string>
#include <cstdlib>
#include <time.h>

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cDonkeyKong::cDonkeyKong( int x, int y, int direction, int difficulty ) : x(x), y(y), doing(cDonkeyKong::DO_WAITING), direction(direction)
{
  // Cargo los sprites
  this->dk_standby.add_image( "images/dk_standby.png", 1000 );
  switch( direction )
  {
    case DIR_RIGHT:
      this->dk_throw.add_image( "images/dk_arm_left.png", 400 );
      this->dk_throw.add_image( "images/dk_arm_right.png", 400 );
      this->dk_throw.add_image( "images/dk_left.png", 400 );
      this->dk_throw.add_image( "images/dk_standby_barrel.png", 400 );
      this->dk_throw.add_image( "images/dk_right.png", 400 );
      this->dk_throw.add_image( "images/dk_standby.png", 400 );
      break;

    case DIR_LEFT:
      this->dk_throw.add_image( "images/dk_arm_right.png", 400 );
      this->dk_throw.add_image( "images/dk_arm_left.png", 400 );
      this->dk_throw.add_image( "images/dk_right.png", 400 );
      this->dk_throw.add_image( "images/dk_standby_barrel.png", 400 );
      this->dk_throw.add_image( "images/dk_left.png", 400 );
      this->dk_throw.add_image( "images/dk_standby.png", 400 );
      break;
  }

  // Seteo el ancho y alto (lo tomo de una de las imagenes, pero podria hardcodearlo)
  this->w = this->dk_standby.w;
  this->h = this->dk_standby.h;

  // Segun la dificultad es la frecuencia con la que lanza barriles
  switch( difficulty )
  {
    case DIF_EASY:
      this->delay_between_barrels = 5000;
      break;
    case DIF_NORMAL:
      this->delay_between_barrels = 4000;
      break;
    case DIF_HARD:
      this->delay_between_barrels = 3000;
      break;
  }

  // Inicializo el seed del rand()
  srand( time( NULL ) );

  // Empiezo a contar el tiempo desde que fue creado para tirar el primer barril
  this->barrel_throw_time = SDL_GetTicks() - this->delay_between_barrels + 500 + ( rand() % 1000 );
}

/******************************************************************************
* Actualiza la animacion e intenta arrojar un barril
*******************************************************************************/
void cDonkeyKong::update( SDL_Surface* level_mask )
{
  // Intenta lanzar un barril (dependiendo de cuando fue la ultima vez que tiró el ultimo)
  if( this->barrel_throw_time + this->delay_between_barrels < SDL_GetTicks() )
  {
    this->doing = cDonkeyKong::DO_THROWING;
    this->barrel_throw_time = SDL_GetTicks() + ( rand() % 500 );
    cBarrel* barrel = new cBarrel();
    int barrel_x = ( this->direction == DIR_RIGHT ) ? this->x + this->w : this->x - barrel->w;
    int barrel_y = this->y + this->h - barrel->h;
    barrel->start( barrel_x, barrel_y, this->direction, 1650 );
    this->obj_barrels.push_back( barrel );
  }

  // Actualiza la posicion de los barriles y elimina los que ya finalizaron
  list<cBarrel*>::iterator p = this->obj_barrels.begin();
  while( p != this->obj_barrels.end() )
  {
    cBarrel* barrel = *p;
    if( barrel->status != cBarrel::STATUS_FINISHED )
    {
      barrel->update( level_mask );
      ++p;
    }
    else
    {
      free( barrel );
      this->obj_barrels.erase( p++ );
    }
  }
}

/******************************************************************************
* Renderiza el objeto en pantalla
*******************************************************************************/
void cDonkeyKong::render( SDL_Surface* screen )
{
  SDL_Rect object_pos;
  object_pos.x = this->x;
  object_pos.y = this->y;

  // Pone la imagen que corresponda
  switch( this->doing )
  {
    case cDonkeyKong::DO_THROWING:
    {
      if( this->dk != &this->dk_throw ) this->dk_throw.restart_animation();
      this->dk = &this->dk_throw;
      if( this->barrel_throw_time + 2000 < SDL_GetTicks() ) this->doing = cDonkeyKong::DO_WAITING;
      break;
    }

    case cDonkeyKong::DO_WAITING:
    {
      this->dk = &this->dk_standby;
      break;
    }
  }
  this->dk->animate( false );
  SDL_BlitSurface( this->dk->get_image(), 0, screen, &object_pos );

  list<cBarrel*>::iterator p = this->obj_barrels.begin();
  while( p != this->obj_barrels.end() )
  {
    cBarrel* barrel = *p;
    barrel->render( screen );
    p++;
  }
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cDonkeyKong::~cDonkeyKong()
{
  list<cBarrel*>::iterator p = this->obj_barrels.begin();
  while( p != this->obj_barrels.end() )
  {
    free( *p );
    p++;
  }
}
