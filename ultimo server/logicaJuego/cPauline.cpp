#include "cPauline.h"

#include "Misc.h"

using namespace std;

/******************************************************************************
* Crea el objeto
*******************************************************************************/
cPauline::cPauline( int x, int y ) : x(x), y(y)
{
  // Cargo los sprites
  this->pauline_right.add_image( "images/pauline_right_1.png", 200 );
  this->pauline_right.add_image( "images/pauline_right_2.png", 200 );
  this->pauline_left.add_image( "images/pauline_left_1.png", 200 );
  this->pauline_left.add_image( "images/pauline_left_2.png", 200 );

  // Seteo la imagen inicial
  this->pauline = &this->pauline_right;
  this->direction = DIR_RIGHT;

  // Actualizo height y width
  this->w = this->pauline->w;
  this->h = this->pauline->h;
}

/******************************************************************************
* Actualiza la posicion
*******************************************************************************/
void cPauline::update( SDL_Surface* level_mask )
{
  this->my_color = get_pixel_color( this->x + this->w/2, this->y + this->h/2, level_mask );

  if( !is_color( this->my_color, C_WIN ) )
  {
    if( this->direction == DIR_RIGHT )
    {
      this->pauline = &this->pauline_left;
      this->direction = DIR_LEFT;
    }
    else
    {
      this->pauline = &this->pauline_right;
      this->direction = DIR_RIGHT;
    }
  }

  if( this->direction == DIR_RIGHT ) this->x = this->x + 1;
  else if( this->direction == DIR_LEFT ) this->x = this->x - 1;
}

/******************************************************************************
* Renderiza el objeto en pantalla
*******************************************************************************/
void cPauline::render( SDL_Surface* screen )
{
  SDL_Rect object_pos;
  object_pos.x = this->x;
  object_pos.y = this->y;
  this->pauline->animate();
  SDL_BlitSurface( this->pauline->get_image(), 0, screen, &object_pos );
}

/******************************************************************************
* Libera los recursos pedidos
*******************************************************************************/
cPauline::~cPauline()
{
}
