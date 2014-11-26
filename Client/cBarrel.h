#ifndef cBarrelH
#define cBarrelH

#include "Misc.h"
#include "cSprite.h"
#include <SDL/SDL_image.h>

class cBarrel
{
  public:
    // Metodos
    cBarrel();
    void start( int x, int y, char direction, int delay );
    void render( SDL_Surface* screen );
    void update( SDL_Surface* mask );
    void update_colors( SDL_Surface* mask );
    bool mask_color( std::string index, int color );
    ~cBarrel();

    // Propiedades
    int x, y;
    int w, h;
    int status;
    int doing;

    // Constantes
    static const char STATUS_ACTIVE = 1;
    static const char STATUS_INACTIVE = 2;
    static const char STATUS_FINISHED = 3;
    static const char DO_WAITING = 1;
    static const char DO_FALLING = 2;
    static const char DO_ROLLING = 3;

  private:
    // Propiedades
    cSprite* barrel;
    cSprite barrel_side;
    cSprite barrel_front;
    int direction;
    int count_fall;
    int delay;
    unsigned int start_tick;
    color colors[4];
};

#endif
