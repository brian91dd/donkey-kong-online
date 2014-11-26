#ifndef cOilH
#define cOilH

#include "cSprite.h"
#include "cFlame.h"
#include <SDL/SDL_image.h>
#include <list>

class cOil
{
  public:
    // Metodos
    cOil( int x, int y, char direction, char difficulty = DIF_NORMAL );
    void render( SDL_Surface* screen );
    void update( SDL_Surface* level_mask );
    ~cOil();

    // Propiedades
    int x, y;
    int w, h;
    std::list<cFlame*> obj_flames;

  private:
    // Propiedades
    cSprite oil;
    char direction;
    unsigned int flames_throw_time;
    int delay_between_flames;
    int cant_flames;
    int max_flames;
};

#endif
