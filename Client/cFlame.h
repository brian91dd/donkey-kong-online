#ifndef cFlameH
#define cFlameH

#include "Misc.h"
#include "cSprite.h"
#include <SDL/SDL_image.h>

class cFlame
{
  public:
    // Metodos
    cFlame();
    void start( int x, int y, char direction );
    void render( SDL_Surface* screen );
    void update( SDL_Surface* mask );
    void update_colors( SDL_Surface* mask );
    bool mask_color( std::string index, int color );
    ~cFlame();

    // Propiedades
    int x, y;
    int w, h;
    char status;

    // Constantes
    static const char STATUS_ACTIVE = 1;
    static const char STATUS_INACTIVE = 2;
    static const char DO_WALKING = 1;
    static const char DO_CLIMBING = 2;

  private:
    cSprite flame;
    int doing;
    char direction;
    color colors[5];
};

#endif
