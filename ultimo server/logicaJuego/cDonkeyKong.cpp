#include "cDonkeyKong.h"

#include "Misc.h"
#include <string>

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cDonkeyKong::cDonkeyKong( int x, int y, char direction, char difficulty ) : x(x), y(y), doing(cDonkeyKong::DO_WAITING), direction(direction)
{
  // Segun la dificultad es la cantidad maxima de barriles que lanza
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

  // Empiezo a contar el tiempo desde que fue creado para tirar el primer barril
  this->barrel_throw_time = SDL_GetTicks() - this->delay_between_barrels + 500;	//quitar SDL TICKS Y PONER UN CLOCK NUESTRO

}

/******************************************************************************
* Crea el objeto
*******************************************************************************/
void cDonkeyKong::update()
{
  // Intenta lanzar un barril (dependiendo de cuando fue la ultima vez que tiró el ultimo)
  if( this->barrel_throw_time + this->delay_between_barrels < SDL_GetTicks() )		//quitar SDL TICKS Y PONER UN CLOCK NUESTRO
  {
    this->dk = &this->dk_throw;
    this->dk->restart_animation();
    this->barrel_throw_time = SDL_GetTicks();	//quitar SDL TICKS Y PONER UN CLOCK NUESTRO
    int i = this->obj_barrels.size();
    this->obj_barrels[i] = new cBarrel();
    int barrel_w = this->obj_barrels[i]->w;
    int barrel_h = this->obj_barrels[i]->h;
    int barrel_x = ( this->direction == DIR_RIGHT ) ? this->x + this->w : this->x - barrel_w;
    int barrel_y = this->y + this->h - barrel_h;
    this->obj_barrels[i]->start( barrel_x, barrel_y, this->direction, 1650 );
  }

  // Actualiza la posicion de los barriles y elimina los que ya finalizaron
  map<int,cBarrel*>::iterator p = this->obj_barrels.begin();
  //printf("Hay %d\n", this->obj_barrels.size());
  while( p != this->obj_barrels.end() )
  {
    cBarrel* barrel = p->second;
    if( barrel->status != cBarrel::STATUS_FINISHED )
    {
      barrel->update();
      ++p;
    }
  }
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cDonkeyKong::~cDonkeyKong()
{
  map<int,cBarrel*>::iterator p = this->obj_barrels.begin();
  while( p != this->obj_barrels.end() )
  {
    free( p->second );
    p++;
  }
}
