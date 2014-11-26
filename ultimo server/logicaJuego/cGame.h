#ifndef cGameH
#define cGameH

#include "Misc.h"
#include "cTextFactory.h"
#include "cPlayer.h"
#include "cSprite.h"
#include "cOil.h"
#include "cPauline.h"
#include "cDonkeyKong.h"
#include <map>
#include <list>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

class cGame
{
  public:
    cGame( );
    void run();
    ~cGame();

    // Teclas
    static const int JUMP = 0;
    static const int UP = 1;
    static const int DOWN = 2;
    static const int RIGHT = 3;
    static const int LEFT = 4;

    // Estados del juego
    static const char STATUS_INIT = 1;
    static const char STATUS_WAITING = 2;
    static const char STATUS_PLAYING = 3;
    static const char STATUS_RESULTS = 4;
    static const char STATUS_TOURNAMENT_RESULTS = 5;
    static const char STATUS_ERROR = 6;
    static const char STATUS_ERROR_TOURNAMENT = 7;
    static const char STATUS_ERROR_GAME = 8;

    // Resultado de la partida
    static const char RES_WIN = 1;
    static const char RES_LOSE = 2;
    static const char RES_TIE = 3;
    static const char RES_UNDEFINED = 4;

    // Tipo de objetos del mapa
    static const char BARREL = 1;
    static const char FLAME = 2;
    static const char DK = 3;
    static const char OIL_BARREL = 4;
    static const char PAULINE = 5;

    // Max frames por segundo
    static const int FPS = 45;

 /* private:
    // Clase que imprime texto en pantalla (SDL)
    cTextFactory writer;*/

    // Algunas propiedades
    int w, h;
    char result_end_game;

    // Jugadores y objetos
    cPlayer* p1;
    cPlayer* p2;
    std::map<int,cDonkeyKong*> obj_donkeykongs;
    std::map<int,cOil*> obj_oils;
    cPauline* pauline;

    // Estado del juego
    char game_status;
    bool running;
    bool exit_cofirm;
    int tick_time;
    int remaining_time;
    int max_time_int;
    std::string wait_time;
    int barrel_throw_time;

    // Datos del servidor de torneo (se cargan con el archivo de config)
    std::string ip;
    int port;

    // Mapeo de teclas con SDL
    int key_up;
    int key_dw;
    int key_rt;
    int key_lt;
    int key_jump;

 /*   // Datos del player que tiene el cliente abierto
    std::string player_id;
    std::string player_name;*/

    // Datos de la partida actual
    std::string p1_id;
    std::string p2_id;
    std::string p1_name;
    std::string p2_name;
    std::string opponent_name;
    std::string max_time;
    std::string max_lives;

    // Teclas que están siendo presionadas
    std::map<std::string,bool> key;

  /*  // Imagenes del juego
    SDL_Surface* screen;
    SDL_Surface* background;
    SDL_Surface* back_black;
    SDL_Surface* back_level;*/
    SDL_Surface* level_mask;
  /*  SDL_Surface* img_life;
    SDL_Surface* img_pauline;*/

   /* // Usado para renderear
    SDL_Rect dstrect;*/

   /* // Funciones de inicializacion
    int get_sdl_key( std::string );*/
    void new_game( int difficulty );

    // Funciones de procesamiento de entrada
  /*  void process_keyboard();
    void process_messages();*/

  /*  // Funciones de rendereo
    void render_init();
    void render_waiting();
    void render_playing();
    void render_results( bool final = false );
    void render_error();
    void render_exit();
    void render_error_game();
    void render_error_tournament();*/

    // Funciones de chequeos y actualizacion
    void update_playing();
    bool check_collision( cPlayer* player );
};

#endif
