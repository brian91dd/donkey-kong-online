#include "cPauline.h"

#include "Misc.h"

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cPauline::cPauline( int x, int y ) : x(x), y(y), direction(DIR_RIGHT)
{
  // Cargo los sprites
  this->pauline_right.add_image( "images/pauline_right_1.png", 200 );
  this->pauline_right.add_image( "images/pauline_right_2.png", 200 );
  this->pauline_left.add_image( "images/pauline_left_1.png", 200 );
  this->pauline_left.add_image( "images/pauline_left_2.png", 200 );

  // Seteo el ancho y alto (lo tomo de una de las imagenes, pero podria hardcodearlo)
  this->w = this->pauline_right.w;
  this->h = this->pauline_right.h;
}

/******************************************************************************
* Actualiza la posicion
*******************************************************************************/
void cPauline::update( SDL_Surface* level_mask )
{
  this->my_color = get_pixel_color( this->x + this->w/2, this->y + this->h/2, level_mask );

  if( !is_color( this->my_color, C_WIN ) )
    this->direction = this->direction == DIR_RIGHT ? DIR_LEFT : DIR_RIGHT;

  switch( this->direction )
  {
    case DIR_LEFT: this->x = this->x - 1; break;
    case DIR_RIGHT: this->x = this->x + 1; break;
  }
}

/******************************************************************************
* Renderiza el objeto en pantalla
*******************************************************************************/
void cPauline::render( SDL_Surface* screen )
{
  SDL_Rect object_pos;
  object_pos.x = this->x;
  object_pos.y = this->y;
  switch( this->direction )
  {
    case DIR_LEFT: this->pauline = &this->pauline_left; break;
    case DIR_RIGHT: this->pauline = &this->pauline_right; break;
  }
  this->pauline->animate();
  SDL_BlitSurface( this->pauline->get_image(), 0, screen, &object_pos );
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cPauline::~cPauline()
{
}
