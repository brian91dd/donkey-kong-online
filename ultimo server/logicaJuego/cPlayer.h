#ifndef CPLAYER_H
#define CPLAYER_H

#include "cBarrel.h"
#include "Misc.h"
#include <string>
#include <SDL/SDL_image.h>

class cPlayer
{
  public:
    int x; // posicion x en el mapa
    int y; // posicion y en el map
    int w; // ancho
    int h; // alto

    char player_number;
    std::string name; // nombre del jugador
    int status; // estado ( ALIVE / DEAD / ALIVE_WIN )
    int doing; // acciones que esta haciendo (WAITING / WALKING / JUMPING / CLIMBING)
    int direction; // direccion (DIR_RIGHT / DIR_LEFT)
    int wins; // cantidad de veces que llego al nido
    int lives;

    cPlayer( char player, std::string name, int x, int y );
    void update( std::map<std::string,bool> key, SDL_Surface* level_mask );
    void update_colors( SDL_Surface* mask );
    void restart();
    void die();
    void win();
    void set_position( int x, int y );
    void move_right();
    void move_left();
    void move_jump();
    void move_climb_up();
    void move_climb_down();
    bool mask_color( std::string index, int color );

    ~cPlayer();

    static const char P1 = 1;
    static const char P2 = 2;

    // Estado del player
    static const char ALIVE = 1;
    static const char DEAD = 2;
    static const char ALIVE_WIN = 3;

    // Acciones
    static const char WAITING = 1;
    static const char WALKING = 2;
    static const char JUMPING = 3;
    static const char CLIMBING = 4;

  private:
    int x_start; // posicion x donde nace el player
    int y_start; // posicion y donde nace el player
    float speed_y;

    color colors[5];

    unsigned long hold_tick; // ticks desde que murio o llego al nido
};

#endif
