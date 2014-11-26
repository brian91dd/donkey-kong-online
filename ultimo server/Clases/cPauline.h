#ifndef cPaulineH
#define cPaulineH

#include "Misc.h"
#include "cSprite.h"
#include <SDL/SDL_image.h>

class cPauline
{
  public:
    // Metodos
    cPauline( int x, int y );
    void render( SDL_Surface* screen );
    void update( SDL_Surface* level_mask );
    ~cPauline();

    // Propiedades
    int x, y;
    int w, h;

  private:
    // Propiedades
    cSprite* pauline;
    cSprite pauline_right;
    cSprite pauline_left;
    char direction;
    color my_color;
};

#endif
