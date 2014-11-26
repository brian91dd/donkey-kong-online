#ifndef cButtonH
#define cButtonH

#include <SDL/SDL.h>

class cButton
{
  public:
    SDL_Rect size;
    bool is_clicked( SDL_Event event );
    void set_size( SDL_Rect size );
};

#endif
