#include "cFlame.h"

#include <string>
#include <stdlib.h>
#include <time.h>

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cFlame::cFlame() : status(cFlame::STATUS_INACTIVE), doing(cFlame::DO_WALKING)
{
  // Cargo los sprites
  this->flame.add_image( "images/flame_left.png", 400 );
  this->flame.add_image( "images/flame_right.png", 400 );

  // Seteo el ancho y alto (lo tomo de una de las imagenes, pero podria hardcodearlo)
  this->w = this->flame.w;
  this->h = this->flame.h;

  // Inicializo el seed del rand()
  srand( time( NULL ) );
}

/******************************************************************************
* Inicia el objeto
*******************************************************************************/
void cFlame::start( int x, int y, char direction )
{
  this->x = x;
  this->y = y;
  this->direction = direction;
  this->status = cFlame::STATUS_ACTIVE;
}

/******************************************************************************
* Renderiza el objeto en pantalla
*******************************************************************************/
void cFlame::render( SDL_Surface* screen )
{
  if( this->status == cFlame::STATUS_ACTIVE )
  {
    SDL_Rect object_pos;
    object_pos.x = this->x;
    object_pos.y = this->y;
    this->flame.animate();
    SDL_BlitSurface( this->flame.get_image(), 0, screen, &object_pos );
  }
}

/******************************************************************************
* Actualiza la posición del objeto en el mapa
*******************************************************************************/
void cFlame::update( SDL_Surface* mask )
{
  if( this->status == cFlame::STATUS_ACTIVE )
  {
    // Actualiza los colores
    this->update_colors( mask );

    // Dependiendo de si estoy caminando o en la escalera...
    switch( this->doing )
    {
      // Si estoy caminando...
      case cFlame::DO_WALKING:
      {
        // La mantengo siempre por encima del piso (sirve para los escaloncitos)
        while( this->mask_color( "UNDER_1", C_FLOOR ) || this->mask_color( "UNDER_1", C_LADDER_TOP ) )
        {
          this->y = this->y - 1;
          this->update_colors( mask );
        }

        // Si a los costados hay verde, cambio de direccion
        if( this->mask_color( "LEFT", C_FLOOR ) || this->mask_color( "RIGHT", C_FLOOR ) )
        {
          if( this->direction == DIR_RIGHT ) this->direction = DIR_LEFT;
          else if( this->direction == DIR_LEFT ) this->direction = DIR_RIGHT;
        }
        // Si debajo hay una escalera, hay una chance que la baje
        else if( this->mask_color( "UNDER_2", C_LADDER_TOP ) && rand() % 30 == 1 )
        {
          this->doing = cFlame::DO_CLIMBING;
          this->direction = DIR_DOWN;
        }
        // Si estoy en la base de una escalera, hay una chance que la suba
        else if( this->mask_color( "UNDER_1", C_LADDER ) && rand() % 15 == 1 )
        {
          this->doing = cFlame::DO_CLIMBING;
          this->direction = DIR_UP;
        }
        // Si debajo hay precipicio, cambio de direccion
        else if( this->mask_color( "UNDER_2", C_EMPTY ) && this->mask_color( "UNDER_3", C_EMPTY ) )
        {
          if( this->direction == DIR_RIGHT ) this->direction = DIR_LEFT;
          else if( this->direction == DIR_LEFT ) this->direction = DIR_RIGHT;
        }
        // Si debajo no hay ni piso ni escalera
        else if( !this->mask_color( "UNDER_2", C_FLOOR ) && !this->mask_color( "UNDER_2", C_LADDER_TOP ) )
        {
          this->y = this->y + 1;
        }

        break;
      }
      // Si estoy en la escalera
      case cFlame::DO_CLIMBING:
      {
        // Si debajo hay una escalera y estaba subiendo, es que terminé de subir
        if( this->mask_color( "UNDER_2", C_LADDER_TOP ) && this->direction == DIR_UP )
        {
          this->doing = cFlame::DO_WALKING;
          this->direction = ( rand() % 2 == 1 ) ? DIR_LEFT : DIR_RIGHT;
        }
        // Si debajo hay piso y estaba bajando, es que terminé de bajar
        else if( this->mask_color( "UNDER_2", C_FLOOR ) && this->direction == DIR_DOWN )
        {
          this->doing = cFlame::DO_WALKING;
          this->direction = ( rand() % 2 == 1 ) ? DIR_LEFT : DIR_RIGHT;
        }

        break;
      }
    }

    // Si está sobre el piso...
    if( this->doing == cFlame::DO_WALKING )
    {
      if( this->direction == DIR_RIGHT ) this->x = this->x + 1;
      else if( this->direction == DIR_LEFT ) this->x = this->x - 1;
    }

    // Si está en una escalera
    else
    {
      if( this->direction == DIR_UP ) this->y = this->y - 1;
      else if( this->direction == DIR_DOWN ) this->y = this->y + 1;
    }
  }
}

/******************************************************************************
* Devuelve true o false si un determinado pixel es del color pasado por parametro
*******************************************************************************/
bool cFlame::mask_color( string index, int col )
{
  if( index == "UNDER_1" ) return is_color( this->colors[0], col );
  else if( index == "UNDER_2" ) return is_color( this->colors[1], col );
  else if( index == "UNDER_3" ) return is_color( this->colors[2], col );
  else if( index == "LEFT" ) return is_color( this->colors[3], col );
  else if( index == "RIGHT" ) return is_color( this->colors[4], col );
  return false;
}

/******************************************************************************
* Actualiza los colores contra los que mapea la posicion en la máscara
*******************************************************************************/
void cFlame::update_colors( SDL_Surface* mask )
{
  this->colors[0] = get_pixel_color( this->x + this->w/2, this->y + this->h, mask ); // UNDER_1
  this->colors[1] = get_pixel_color( this->x + this->w/2, this->y + this->h + 1, mask ); // UNDER_2
  this->colors[2] = get_pixel_color( this->x + this->w/2, this->y + this->h + 10, mask ); // UNDER_3
  this->colors[3] = get_pixel_color( this->x, this->y, mask ); // LEFT-UP
  this->colors[4] = get_pixel_color( this->x + this->w, this->y, mask ); // RIGHT-UP
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cFlame::~cFlame()
{
}
