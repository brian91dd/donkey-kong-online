#include "cSprite.h"

#include "Misc.h"

using namespace std;

/******************************************************************************
* Constructor
*******************************************************************************/
cSprite::cSprite() :w(0), h(0), current_frame(0), old_tick(0)
{
}

/******************************************************************************
* Agrega una imagen al sprite
*******************************************************************************/
void cSprite::add_image( string image, int time )
{
  tFrame frame;
  frame.image = load_image( image.c_str(), true );
  frame.time = time;
  if( this->w == 0 ) this->w = frame.image->w;
  if( this->h == 0 ) this->h = frame.image->h;
  this->frames[this->frames.size()] = frame;
}

/******************************************************************************
* Obtiene el frame que corresponde
*******************************************************************************/
SDL_Surface* cSprite::get_image()
{
  return this->frames[this->current_frame].image;
}

/******************************************************************************
* Anima el sprite
*******************************************************************************/
void cSprite::animate( bool loop )
{
  if( this->old_tick + this->frames[this->current_frame].time > SDL_GetTicks() ) return;
  this->old_tick = SDL_GetTicks();
  this->current_frame++;
  if( this->current_frame >= this->frames.size() )
  {
    if( loop ) this->current_frame = 0;
    else this->current_frame--;
  }
}

/******************************************************************************
* Reinicia la animacion (sirve para cuando se ejecuta con loop = false )
*******************************************************************************/
void cSprite::restart_animation()
{
  this->current_frame = 0;
  this->old_tick = SDL_GetTicks();
}

cSprite::~cSprite()
{
  map<int,tFrame>::iterator it = this->frames.begin();
  while ( it != this->frames.end() )
  {
    free( it->second.image );
    it++;
  }
}
