#ifndef cDonkeyKongH
#define cDonkeyKongH

#include "cSprite.h"
#include "cBarrel.h"
#include <SDL/SDL_image.h>
#include <list>

class cDonkeyKong
{
  public:
    // Metodos
    cDonkeyKong( int x, int y, char direction, char difficulty );
    void render( SDL_Surface* screen );
    void update( SDL_Surface* level_mask );
    ~cDonkeyKong();

    // Propiedades
    int x, y;
    int w, h;
    std::list<cBarrel*> obj_barrels;

    // Constantes
    static const char DO_WAITING = 1;
    static const char DO_THROWING = 2;


    // Propiedades
    cSprite* dk;
    cSprite dk_standby;
    cSprite dk_throw;
    char doing;
    int direction;
    unsigned int barrel_throw_time;
    int delay_between_barrels;
};

#endif
