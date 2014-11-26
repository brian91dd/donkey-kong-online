#include "cTextFactory.h"
#include "md5.h"

#include <sstream>

using namespace std;

void cTextFactory::init( SDL_Surface* screen, std::string font )
{
    this->font = font;
    this->screen = screen;
}

void cTextFactory::printf( int x, int y, std::string text, int size, int color, char align, bool button )
{
  // si no esta cargada la font en ese tamaño...
  if( this->fonts.find( size ) == this->fonts.end( ) )
  {
    this->fonts[size] = TTF_OpenFont( this->font.c_str(), size );
  }
  // genero la key
  std::ostringstream tmp;
  tmp << (size*color);
  string key = tmp.str() + "-" + text;
  MD5 md5( key );
  string md5_key = md5.hexdigest();
  // busca si esta creado el string (con ese color y tamaño)
  if( this->strings.find( md5_key ) == this->strings.end( ) )
  {
    int w = 0, h = 0;
    SDL_Color text_color = { (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF };
    this->strings[md5_key] = TTF_RenderText_Blended( this->fonts[size], text.c_str(), text_color );
    TTF_SizeText( this->fonts[size], text.c_str(), &w, &h );
    this->widths[md5_key] = w;
    this->height[md5_key] = h;
  }
  // lo renderiza en pantalla
  switch( align )
  {
    case cTextFactory::LEFT:
    {
      this->pos.x = x;
      this->pos.y = y;
      break;
    }
    case cTextFactory::RIGHT:
    {
      this->pos.x = x - this->widths[md5_key];
      this->pos.y = y;
      break;
    }
    case cTextFactory::CENTER:
    {
      this->pos.x = x - ( this->widths[md5_key] / 2 );
      this->pos.y = y;
      break;
    }
  }

  SDL_Rect rect;
  SDL_Rect rect_base;

  if( button )
  {
    // base
    rect_base.x = this->pos.x-15;
    rect_base.y = this->pos.y;
    rect_base.w = this->widths[md5_key]+30;
    rect_base.h = this->height[md5_key]+5;

    // borde negro
    rect.x = rect_base.x-3;
    rect.y = rect_base.y+1;
    rect.w = rect_base.w+2;
    rect.h = rect_base.h+2;
    SDL_FillRect( this->screen, &rect, SDL_MapRGB( this->screen->format, 0, 0, 0 ) );

    // naranja
    rect.x = rect_base.x-2;
    rect.y = rect_base.y+2;
    rect.w = rect_base.w;
    rect.h = rect_base.h;
    SDL_FillRect( this->screen, &rect, SDL_MapRGB( this->screen->format, 248, 120, 31 ) );

    // borde negro
    rect.x = rect_base.x-1;
    rect.y = rect_base.y-1;
    rect.w = rect_base.w+2;
    rect.h = rect_base.h+2;
    SDL_FillRect( this->screen, &rect, SDL_MapRGB( this->screen->format, 0, 0, 0 ) );

    // naranja
    SDL_FillRect( this->screen, &rect_base, SDL_MapRGB( this->screen->format, 255, 237, 7 ) );
  }

  SDL_BlitSurface( this->strings[md5_key], 0, this->screen, &pos );
}

/******************************************************************************
* Libera todos los recursos pedidos
*******************************************************************************/
cTextFactory::~cTextFactory()
{
  // libera los surface creados
  map<string,SDL_Surface*>::iterator it2 = this->strings.begin();
  while( it2 != this->strings.end() )
  {
    SDL_FreeSurface( it2->second );
    it2++;
  }

  // cierra las fonts abiertas
  map<int,TTF_Font*>::iterator it1 = this->fonts.begin();
  while( it1 != this->fonts.end() )
  {
    TTF_CloseFont( it1->second );
    it1++;
  }
}
