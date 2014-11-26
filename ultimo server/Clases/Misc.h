#ifndef MiscH
#define MiscH

#include <sstream>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define GRAVITATION 0.5

#define C_FLOOR 1
#define C_EMPTY 2
#define C_LADDER 3
#define C_LADDER_TOP 4
#define C_WIN 5

#define DIR_LEFT 1
#define DIR_RIGHT 2
#define DIR_UP 1
#define DIR_DOWN 2

#define DIF_EASY 1
#define DIF_NORMAL 2
#define DIF_HARD 3

template<typename T> std::string to_str( T var )
{
  std::ostringstream tmp;
  tmp << var;
  return tmp.str();
}

typedef struct color {
  Uint8 r;
  Uint8 g;
  Uint8 b;
} color;

inline Uint32 get_pixel_index( SDL_Surface *surface, int x, int y )
{
  int bpp = surface->format->BytesPerPixel;
  Uint8* p = (Uint8*) surface->pixels + y * surface->pitch + x * bpp;
  switch( bpp )
  {
    case 1: return *p;
    case 2: return * (Uint16*) p;
    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) return p[0] << 16 | p[1] << 8 | p[2];
      else return p[0] | p[1] << 8 | p[2] << 16;
    case 4: return * (Uint32*) p;
    default: return 0;
  }
}

inline color get_pixel_color( int x, int y, SDL_Surface* map )
{
  color color;
  if( x > 0 && y > 0 && x < map->w && y < map->h )
  {
    if( SDL_MUSTLOCK( map ) ) SDL_LockSurface( map );
    SDL_GetRGB( get_pixel_index( map, x, y ), map->format, &color.r, &color.g, &color.b );
    if( SDL_MUSTLOCK( map ) ) SDL_UnlockSurface( map );
  }
  else
  {
    color.r = 0;
    color.g = 0;
    color.b = 0;
  }
  return color;
}

inline bool is_color( const color& col, int type )
{
  switch( type )
  {
    case C_FLOOR: if( col.r == 0 && col.g == 255 && col.b == 0 ) return true; break;
    case C_EMPTY: if( col.r == 0 && col.g == 0 && col.b == 0 ) return true; break;
    case C_LADDER: if( col.r == 0 && col.g == 0 && col.b == 255 ) return true; break;
    case C_LADDER_TOP: if( col.r == 0 && col.g == 255 && col.b == 255 ) return true; break;
    case C_WIN: if( col.r == 255 && col.g == 0 && col.b == 0 ) return true; break;
  }
  return false;
}

inline bool compare_color( const color& a, const color& b )
{
  if( a.r == b.r && a.g == b.g && a.b == b.b ) return true;
  else return false;
}

inline void draw_pixel_nolock( SDL_Surface * surface, int x, int y, Uint32 color )
{
  Uint8 * pixel = (Uint8*)surface->pixels;
  pixel += (y * surface->pitch) + (x * sizeof(Uint16));
  *((Uint16*)pixel) = color & 0xFFFF;
}

inline void draw_pixel( SDL_Surface * surface, int x, int y, Uint32 color )
{
  if( SDL_MUSTLOCK(surface) ) SDL_LockSurface(surface);
  draw_pixel_nolock(surface, x, y, color);
  if( SDL_MUSTLOCK(surface) ) SDL_UnlockSurface(surface);
}

inline SDL_Surface* load_image( std::string image_path, bool alpha )
{
  SDL_Surface* surface = IMG_Load( image_path.c_str() );
  SDL_Surface* optimized_surface = alpha ? SDL_DisplayFormatAlpha( surface ) : SDL_DisplayFormat( surface );
  SDL_FreeSurface( surface );
  return optimized_surface;
}

inline SDL_Surface* load_image( std::string image_path )
{
  return load_image( image_path, false );
}

#endif
