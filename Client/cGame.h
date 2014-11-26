#ifndef cGameH
#define cGameH

#include "Misc.h"
#include "cTextFactory.h"
#include "cButton.h"
#include "cPlayer.h"
#include "cSprite.h"
#include "cOil.h"
#include "cPauline.h"
#include "cDonkeyKong.h"
#include "cServerConn.h"
#include "cSafeQueue.h"
#include "cInstruction.h"
#include "cPlayerStats.h"
#include <map>
#include <list>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

class cGame
{
  public:
    cGame( std::string player_name );
    void run();
    ~cGame();

    // Estados del juego
    static const char STATUS_INIT = 1;
    static const char STATUS_PLAYING = 2;
    static const char STATUS_RESULTS = 3;
    static const char STATUS_ERROR = 4;
    static const char STATUS_ERROR_TOURNAMENT = 5;
    static const char STATUS_ERROR_GAME = 6;

    // Resultado de la partida
    static const char RES_WIN = 1;
    static const char RES_LOSE = 2;
    static const char RES_UNDEFINED = 3;

    // Max frames por segundo
    static const int FPS = 45;

  private:
    // Clase que imprime texto en pantalla (SDL)
    cTextFactory writer;
    cTextFactory writer_arcade;
    cTextFactory writer_off;

    // Coneccion con el server y cola de mensajes recibidos
    cServerConn server;
    cSafeQueue<cInstruction> messages;

    // Algunas propiedades
    int w, h;
    char result_end_game;

    // Jugadores y objetos
    cPlayer* p1;
    cPlayer* p2;
    std::map<int,cDonkeyKong*> obj_donkeykongs;
    std::map<int,cOil*> obj_oils;
    std::vector<cBarrel*> obj_barrels;
    std::vector<cFlame*> obj_flames;
    cPauline* pauline;

    // Estadisticas del torneo
    std::vector<cPlayerStats> stats;
    bool tournament_in_progress;

    // Estado del juego
    char game_status;
    bool running;
    bool exit_cofirm;
    int tick_time;
    std::string wait_time;

    // Datos que se cargan con el archivo de config (ip, puerto y teclas)
    std::string ip;
    int port;
    int key_up;
    int key_dw;
    int key_rt;
    int key_lt;
    int key_jump;
    bool multiplayer;

    // Datos del player que abrio el cliente (pasado por parametro)
    std::string player_name;

    // Datos de la partida actual
    std::string p1_name;
    std::string p2_name;
    std::string max_lives;
    std::string max_rescues;
    int difficulty;

    // Teclas que están siendo presionadas
    std::map<std::string,bool> key;

    // Imagenes del juego
    SDL_Surface* screen;
    SDL_Surface* background;
    SDL_Surface* back_black;
    SDL_Surface* back_level;
    SDL_Surface* back_init;
    SDL_Surface* back_stats;
    SDL_Surface* back_info;
    SDL_Surface* level_mask;
    SDL_Surface* img_life;
    SDL_Surface* img_pauline;

    // Botones del juego
    cButton btn_play_singleplayer;
    cButton btn_play_multiplayer;
    cButton btn_exit;
    cButton btn_yes;
    cButton btn_no;

    // Usado para renderear
    SDL_Rect dstrect;

    // Funciones de inicializacion
    int get_sdl_key( std::string );
    void new_game( std::vector<map_object>, bool multiplayer = false );
    void free_resources();

    // Funciones de procesamiento de entrada
    void process_keyboard();
    void process_messages();

    // Funciones de chequeos actualizacion y render
    void render();
    void render_results( bool final = false );
    void update_playing();
    bool check_collision( cPlayer* player );
};

#endif
