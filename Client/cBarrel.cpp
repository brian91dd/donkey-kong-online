#include "cBarrel.h"

#include <string>
#include <stdlib.h>
#include <time.h>

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cBarrel::cBarrel() : status(cBarrel::STATUS_INACTIVE), doing(cBarrel::DO_ROLLING), count_fall(0)
{
  // Cargo los sprites
  this->barrel_side.add_image( "images/barrel_side_1.png" );
  this->barrel_side.add_image( "images/barrel_side_2.png" );
  this->barrel_front.add_image( "images/barrel_front_1.png" );
  this->barrel_front.add_image( "images/barrel_front_2.png" );

  // Seteo el ancho y alto (lo tomo de una de las imagenes, pero podria hardcodearlo)
  this->w = this->barrel_side.w;
  this->h = this->barrel_side.h;

  // Inicializo el seed del rand()
  srand( time( NULL ) );
}

/******************************************************************************
* Inicia el objeto
*******************************************************************************/
void cBarrel::start( int x, int y, char direction, int delay )
{
  this->x = x;
  this->y = y;
  this->direction = direction;
  this->delay = delay;
  this->start_tick = SDL_GetTicks();
}

/******************************************************************************
* Renderiza el objeto en pantalla
*******************************************************************************/
void cBarrel::render( SDL_Surface* screen )
{
  if( this->status == cBarrel::STATUS_ACTIVE )
  {
    SDL_Rect object_pos;
    object_pos.x = this->x;
    object_pos.y = this->y;
    // Cambia a la imagen que corresponda
    switch( this->doing )
    {
      case cBarrel::DO_FALLING: this->barrel = &this->barrel_front; break;
      case cBarrel::DO_ROLLING: this->barrel = &this->barrel_side; break;
    }
    this->barrel->animate();
    SDL_BlitSurface( this->barrel->get_image(), 0, screen, &object_pos );
  }
}

/******************************************************************************
* Actualiza la posición del objeto en el mapa
*******************************************************************************/
void cBarrel::update( SDL_Surface* mask )
{
  switch( this->status )
  {
    case cBarrel::STATUS_INACTIVE:
    {
      if( this->start_tick + this->delay < SDL_GetTicks() ) this->status = cBarrel::STATUS_ACTIVE;
      break;
    }

    case cBarrel::STATUS_ACTIVE:
    {
      // Actualiza los colores
      this->update_colors( mask );

      // Chequea si debo seguir rendereandolo
      if( ( this->mask_color( "LEFT", C_FLOOR ) || this->mask_color( "RIGHT", C_FLOOR ) ) && this->doing == cBarrel::DO_ROLLING )
      {
        this->status = cBarrel::STATUS_FINISHED;
        return;
      }

      // Si debajo hay piso
      if( this->mask_color( "UNDER_1", C_FLOOR ) )
      {
        this->doing = cBarrel::DO_ROLLING;
        this->count_fall = 0;
      }

      // Si debajo hay una escalera
      else if( this->mask_color( "UNDER_1", C_LADDER_TOP )  )
      {
        if( this->doing == cBarrel::DO_ROLLING )
        {
          // Chances de caer por la escalera cortada ( 1/45 por cada pixel )
          if( !this->mask_color( "UNDER_2", C_LADDER ) && rand() % 30 == 1 ) this->doing = cBarrel::DO_FALLING;

          // Chances de caer por la escalera completa ( 1/35 por cada pixel )
          else if( this->mask_color( "UNDER_2", C_LADDER ) && rand() % 20 == 1 ) this->doing = cBarrel::DO_FALLING;
        }
        else this->y = this->y + 2;
      }

      // Si debajo no hay nada, entonces cae
      else
      {
        this->y = this->y + 2;
        this->count_fall++;

        // Si ya estuvo cayendo durante 6 ciclos, asumo que esta cayendo en caida libre, cambio la animacion y la direccion
        if( count_fall == 6 )
        {
          this->doing = cBarrel::DO_FALLING;
          if( this->direction == DIR_RIGHT ) this->direction = DIR_LEFT;
          else this->direction =DIR_RIGHT;
        }
      }

      // Si esta sobre el piso rodando...
      if( this->doing == cBarrel::DO_ROLLING )
      {
        if( this->direction == DIR_RIGHT ) this->x = this->x + 2;
        else if( this->direction == DIR_LEFT ) this->x = this->x - 2;
      }
      break;
    }
  }
}

/******************************************************************************
* Devuelve true o false si un determinado pixel es del color pasado por parametro
*******************************************************************************/
bool cBarrel::mask_color( string index, int col )
{
  if( index == "UNDER_1" ) return is_color( this->colors[0], col );
  else if( index == "UNDER_2" ) return is_color( this->colors[1], col );
  else if( index == "LEFT" ) return is_color( this->colors[2], col );
  else if( index == "RIGHT" ) return is_color( this->colors[3], col );
  return false;
}

/******************************************************************************
* Actualiza los colores contra los que mapea la posicion en la máscara
*******************************************************************************/
void cBarrel::update_colors( SDL_Surface* mask )
{
  this->colors[0] = get_pixel_color( this->x + this->w/2, this->y + this->h + 1, mask ); // UNDER_1
  this->colors[1] = get_pixel_color( this->x + this->w/2, this->y + this->h + 40, mask ); // UNDER_2
  this->colors[2] = get_pixel_color( this->x, this->y, mask ); // LEFT-UP
  this->colors[3] = get_pixel_color( this->x + this->w, this->y, mask ); // RIGHT-UP
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cBarrel::~cBarrel()
{
}
