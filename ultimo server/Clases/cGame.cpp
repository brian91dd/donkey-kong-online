#include "cGame.h"

#include <SDL/SDL.h>
#include <unistd.h>

using namespace std;

cGame::cGame( string player1_name, string player2_name )
:
result_end_game(cGame::RES_UNDEFINED),p1(NULL), p2(NULL),
game_status(cGame::STATUS_PLAYING), running(true), exit_cofirm(false),
tick_time(0), player1_name(player1_name), player2_name(player2_name)
{
  // Inicializa que botones están presionados en false
  this->key1["JUMP"] = false;
  this->key1["UP"] = false;
  this->key1["DOWN"] = false;
  this->key1["RIGHT"] = false;
  this->key1["LEFT"] = false;

  this->key2["JUMP"] = false;
  this->key2["UP"] = false;
  this->key2["DOWN"] = false;
  this->key2["RIGHT"] = false;
  this->key2["LEFT"] = false;

  // Carga la configuracion
  FILE *fr;
  char line[1000];
  string str_line, param, value, keys;
  unsigned int i;
  fr = fopen( "game.cfg", "rt" );
  if( fr == NULL ) printf( "Can't open config file" );
  while( fgets( line, sizeof( line ), fr ) != NULL )
  {
    str_line = line;
    if( ( i = str_line.find( "=" ) ) != string::npos )
    {
      param = str_line.substr( 0, i );
      value = str_line.substr( i+1, str_line.size()-param.size()-2 );
      if( param == "ip" ) this->ip = value;
      if( param == "port" ) this->port = atoi( value.c_str() );
      if( param == "keys" ) keys = value;
    }
  }
  fclose( fr );

  // Carga las keys
  this->key_up = this->get_sdl_key( keys.substr( 0, keys.find(",") ) );
  keys = keys.substr( keys.find(",") + 1 );
  this->key_dw = this->get_sdl_key( keys.substr( 0, keys.find(",") ) );
  keys = keys.substr( keys.find(",") + 1 );
  this->key_rt = this->get_sdl_key( keys.substr( 0, keys.find(",") ) );
  keys = keys.substr( keys.find(",") + 1 );
  this->key_lt = this->get_sdl_key( keys.substr( 0, keys.find(",") ) );
  keys = keys.substr( keys.find(",") + 1 );
  this->key_jump = this->get_sdl_key( keys );

  // Inicia el SDL
  if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) printf( "Unable to init SDL: %s\n", SDL_GetError() );

  // Crea la ventana
  this->screen = SDL_SetVideoMode( 800, 600, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
  if ( !this->screen ) printf( "Unable to set 800x600 video: %s\n", SDL_GetError() );

  // Inicializa las fonts TTF
  if( TTF_Init() == -1 ) printf( "Unable to init TTF: %s\n", SDL_GetError() );

  // Carga los fondos que voy a usar
  this->back_init = load_image( "images/back_black.png" );
  this->back_black = load_image( "images/back_black.png" );
  this->back_level = load_image( "images/back_level.png" );
  this->level_mask = load_image( "images/level_mask.png" );
  this->background = this->back_level;

  // Carga las imagenes que voy a usar
  this->img_life = load_image( "images/life.png", true );
  this->img_pauline = load_image( "images/pauline.png", true );

  this->h = this->background->h;
  this->w = this->background->w;
  this->writer.init( this->screen, "fonts/motor.ttf" );
  this->writer_arcade.init( this->screen, "fonts/arcade.ttf" );
}

/******************************************************************************
* Crea un nuevo juego
*******************************************************************************/
void cGame::new_game( int difficulty )
{
  free( this->p1 );
  free( this->p2 );
  this->difficulty = difficulty;
  this->p1 = NULL;
  this->p2 = NULL;
  this->tick_time = 0;
  this->result_end_game = cGame::RES_UNDEFINED;

  // Creo a Pauline
  this->pauline = new cPauline( 379, 144 );

  // Creo los Donkey Kong
  this->obj_donkeykongs[0] = new cDonkeyKong( 332, 211, DIR_LEFT, difficulty );
  this->obj_donkeykongs[1] = new cDonkeyKong( 443, 211, DIR_RIGHT, difficulty );

  // Creo los barriles de aceite
  this->obj_oils[0] = new cOil( 392, 530, DIR_LEFT, difficulty );
  this->obj_oils[1] = new cOil( 408, 530, DIR_RIGHT, difficulty );

  // Creo los players
  this->p1 = new cPlayer( cPlayer::P1, this->player1_name, 349, 536 );
  this->p2 = new cPlayer( cPlayer::P2, this->player2_name, 456, 536 );
}

/******************************************************************************
* Carga todos los datos necesarios del mapa y comienza a correr el juego
*******************************************************************************/
void cGame::run()
{
  this->new_game( DIF_HARD );

  unsigned int interval = 1000000 / cGame::FPS;

  send_new_game();

  while( this->running )
  {
	send_update_map();

    // procesa la info que se haya obtenido del teclado
    //this->process_keyboard();

    // procesa la info que haya llegado por sockets
    this->process_messages();

    // limpia la pantalla
    SDL_FillRect( this->screen, 0, SDL_MapRGB( this->screen->format, 0, 0, 0 ) );

    switch( this->game_status )
    {
      /************************************************************************************
      * PANTALLA INICIAL (antes de presionar PLAY)
      ************************************************************************************/
      case cGame::STATUS_INIT:
      {
        this->render_init();
        break;
      }

      /************************************************************************************
      * PANTALLA ESPERANDO JUGADORES (una vez que presionamos PLAY)
      ************************************************************************************/
      case cGame::STATUS_WAITING:
      {
        this->render_waiting();
        break;
      }

      /************************************************************************************
      * PANTALLA DEL JUEGO (mientras estamos jugando)
      ************************************************************************************/
      case cGame::STATUS_PLAYING:
      {
        this->update_playing();
        this->render_playing();
        break;
      }

      /************************************************************************************
      * PANTALLA DEL RESULTADOS PARCIALES (antes y despues de cada partida)
      ************************************************************************************/
      case cGame::STATUS_RESULTS:
      {
        this->render_results();
        break;
      }

      /************************************************************************************
      * PANTALLA DEL RESULTADOS FINALES (terminamos todas las partidas)
      ************************************************************************************/
      case cGame::STATUS_TOURNAMENT_RESULTS:
      {
        this->render_results( true );
        break;
      }

      /************************************************************************************
      * PANTALLA DE ERROR (en caso de no poder conectar con el game server)
      ************************************************************************************/
      case cGame::STATUS_ERROR:
      {
        this->render_error();
        break;
      }

      /************************************************************************************
      * PANTALLA DE ERROR (en caso de no poder conectar con el game server)
      ************************************************************************************/
      case cGame::STATUS_ERROR_TOURNAMENT:
      {
        this->render_error_tournament();
        break;
      }

      /************************************************************************************
      * PANTALLA DE ERROR (en caso de no poder conectar con el game server)
      ************************************************************************************/
      case cGame::STATUS_ERROR_GAME:
      {
        this->render_error_game();
        break;
      }
    }

    // si el flag esta activado, muestro la pantalla de salida
    if( this->exit_cofirm ) this->render_exit();

    SDL_Flip( this->screen );
    usleep( interval );
  }

  return;
}

/******************************************************************************
* LOGICA DEL JUEGO
*******************************************************************************/
void cGame::update_playing()
{
  // Actualizo la posicion del player


  // Actualizo la posicion de Pauline
  this->pauline->update( this->level_mask );

  // Actualizo posiciones y acciones de los donkey kong (y sus barriles)
  map<int,cDonkeyKong*>::iterator p = this->obj_donkeykongs.begin();
  while( p != this->obj_donkeykongs.end() )
  {
    cDonkeyKong* donkeykong = p->second;
    donkeykong->update( this->level_mask );
    p++;
  }

  // Actualizo los barriles de aceite y sus llamas
  map<int,cOil*>::iterator q = this->obj_oils.begin();
  while( q != this->obj_oils.end() )
  {
    cOil* cOil = q->second;
    cOil->update( this->level_mask );
    q++;
  }

  // Chequeo colisiones con los barriles o llamas
  if( this->p1->status == cPlayer::ALIVE )
    if( this->check_collision( this->p1 ) ) this->p1->die();

  // Chequeo colisiones con los barriles o llamas
  if( this->p2->status == cPlayer::ALIVE )
    if( this->check_collision( this->p2 ) ) this->p2->die();
}

/******************************************************************************
* Chequea colisiones
*******************************************************************************/
bool cGame::check_collision( cPlayer* player )
{
  // chequeo colisiones contra todos los barriles de todos los donkey kong
  map<int,cDonkeyKong*>::iterator itdk = this->obj_donkeykongs.begin();
  while( itdk != this->obj_donkeykongs.end() )
  {
    cDonkeyKong* donkeykong = itdk->second;
    if( player->y+player->h <= donkeykong->y ) goto SafeDonkeyKong;
    if( player->y >= donkeykong->y+donkeykong->h ) goto SafeDonkeyKong;
    if( player->x+player->w <= donkeykong->x ) goto SafeDonkeyKong;
    if( player->x >= donkeykong->x+donkeykong->w ) goto SafeDonkeyKong;
    return true;

    SafeDonkeyKong:
    list<cBarrel*>* obj_barrels = &donkeykong->obj_barrels;
    list<cBarrel*>::iterator p = obj_barrels->begin();
    while( p != obj_barrels->end() )
    {
      cBarrel* barrel = *p;
      p++;
      // sin colision contra objeto... paso al siguiente
      if( player->y+player->h <= barrel->y ) continue;
      if( player->y >= barrel->y+barrel->h ) continue;
      if( player->x+player->w <= barrel->x ) continue;
      if( player->x >= barrel->x+barrel->w ) continue;
      return true;
    }
    itdk++;
  }

  // chequeo colisiones contra todas las llamas de todos los barriles de aceite
  map<int,cOil*>::iterator itoil = this->obj_oils.begin();
  while( itoil != this->obj_oils.end() )
  {
    cOil* oil = itoil->second;
    if( player->y+player->h <= oil->y ) goto SafeOil;
    if( player->y >= oil->y+oil->h ) goto SafeOil;
    if( player->x+player->w <= oil->x ) goto SafeOil;
    if( player->x >= oil->x+oil->w ) goto SafeOil;
    return true;

    SafeOil:
    list<cFlame*>* obj_flames = &oil->obj_flames;
    list<cFlame*>::iterator p = obj_flames->begin();
    while( p != obj_flames->end() )
    {
      cFlame* flame = *p;
      p++;
      // sin colision contra objeto... paso al siguiente
      if( player->y+player->h <= flame->y ) continue;
      if( player->y >= flame->y+flame->h ) continue;
      if( player->x+player->w <= flame->x ) continue;
      if( player->x >= flame->x+flame->w ) continue;
      return true;
    }
    itoil++;
  }

  return false;
}

/******************************************************************************
* Procesa la informacion que haya llegado desde el teclado
*******************************************************************************/
// void cGame::process_keyboard()
// {
//   SDL_Event event;

//   while( SDL_PollEvent( &event ) )
//   {
//     if( event.type == SDL_QUIT || ( event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) )
//     {
//       this->running = false;
//       //this->exit_cofirm = true;
//       break;
//     }

//     if( !this->exit_cofirm )
//     {
//       switch( this->game_status )
//       {
//         case cGame::STATUS_INIT:
//         {
//           if( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1 )
//           {
//             if( event.button.x > 240 && event.button.x < 240 + 184 )
//             {
//               if( event.button.y > 360 && event.button.y < 360 + 58 )
//               {
//                 /*
//                 // se conecta al servidor de torneo
//                 tournament_conn.connect( this->ip, this->port );
//                 tournament_conn.start( ( void* ) &this->messages );
//                 tournament_conn.send( "PLAYER_NAME " + this->player_name );
//                 this->game_status = cGame::STATUS_WAITING;
//                 */
//               }
//             }
//           }
//           break;
//         }

//         case cGame::STATUS_PLAYING:
//         {
//           if( event.type == SDL_KEYDOWN )
//           {
//             if( event.key.keysym.sym == this->key_up ) this->key["UP"] = true;
//             else if( event.key.keysym.sym == this->key_dw ) this->key["DOWN"] = true;
//             else if( event.key.keysym.sym == this->key_rt ) this->key["RIGHT"] = true;
//             else if( event.key.keysym.sym == this->key_lt ) this->key["LEFT"] = true;
//             else if( event.key.keysym.sym == this->key_jump ) this->key["JUMP"] = true;
//           }
//           else if( event.type == SDL_KEYUP )
//           {
//             if( event.key.keysym.sym == this->key_up ) this->key["UP"] = false;
//             else if( event.key.keysym.sym == this->key_dw ) this->key["DOWN"] = false;
//             else if( event.key.keysym.sym == this->key_rt ) this->key["RIGHT"] = false;
//             else if( event.key.keysym.sym == this->key_lt ) this->key["LEFT"] = false;
//             else if( event.key.keysym.sym == this->key_jump ) this->key["JUMP"] = false;
//           }
//         }
//       }
//     }
//     else
//     {
//       if( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1 )
//       {
//         if( event.button.x > 236 && event.button.x < 296 )
//         {
//           if( event.button.y > 270 && event.button.y < 304 )
//           {
//             this->exit_cofirm = false;
//           }
//         }
//         if( event.button.x > 504 && event.button.x < 561 )
//         {
//           if( event.button.y > 270 && event.button.y < 304 )
//           {
//             this->running = false;
//           }
//         }
//       }
//     }
//   }
// }

/******************************************************************************
* Procesa la informacion que haya llegado desde los sockets
*******************************************************************************/
void cGame::process_messages()
{
  /*
  while( !this->messages.is_empty() )
  {
    cInstruction inst = this->messages.pop();
    string action = inst.get_action();

    if( action == "WAIT_TIME" )
    {
      this->wait_time = inst.get_param(1);
      continue;
    }
    if( action == "DISCONNECT" )
    {
      this->tournament_conn.disconnect();
      continue;
    }
    if( action == "PLAYER_ID" )
    {
      this->player_id = inst.get_param(1);
      continue;
    }
    if( action == "STATS" )
    {
      this->stats.clear();
      for( unsigned int i = 1; i <= inst.size_params(); i++ )
      {
        string player_str = inst.get_param(i);
        vector<string> player_vtr= inst.split( player_str, ',' );
        cPlayerStats player_stats;
        int id = atoi( player_vtr[0].c_str() );
        player_stats.name = player_vtr[1];
        player_stats.wins = player_vtr[2];
        player_stats.losses = player_vtr[3];
        player_stats.status = player_vtr[4];
        this->stats[id] = player_stats;
      }
      continue;
    }
    if( action == "NEW_GAME" )
    {
      int port = atoi( inst.get_param(1).c_str() );
      this->max_time = inst.get_param(2);
      this->max_time_int = atoi( inst.get_param(2).c_str() );
      this->max_lives = inst.get_param(3);
      this->p1_id = inst.get_param(4);
      this->p1_name = inst.get_param(5);
      this->p2_id = inst.get_param(6);
      this->p2_name = inst.get_param(7);
      this->opponent_name = ( this->p1_id == this->player_id ) ? this->p2_name : this->p1_name;
      this->game_status = cGame::STATUS_RESULTS;
      this->wait_time = "--";
      this->new_game( port );
      continue;
    }
    if( action == "SHOW_STATS" )
    {
      this->game_status = cGame::STATUS_TOURNAMENT_RESULTS;
      continue;
    }
    if( action == "INIT_GAME" )
    {
      this->game_status = cGame::STATUS_PLAYING;
      this->tick_time = SDL_GetTicks();
      continue;
    }
    if( action == "YOU_WIN" )
    {
      this->result_end_game = cGame::RES_WIN;
      this->tournament_conn.send( "MATCH_FINISHED" );
      this->game_conn->disconnect();
      continue;
    }
    if( action == "YOU_LOSE" )
    {
      this->result_end_game = cGame::RES_LOSE;
      this->tournament_conn.send( "MATCH_FINISHED" );
      this->game_conn->disconnect();
      continue;
    }
    if( action == "TIE" )
    {
      this->result_end_game = cGame::RES_TIE;
      this->tournament_conn.send( "MATCH_FINISHED" );
      this->game_conn->disconnect();
      continue;
    }
    if( action == "TOURNAMENT_DOWN" )
    {
      this->game_status = cGame::STATUS_ERROR_TOURNAMENT;
      continue;
    }
    if( action == "GAME_DOWN" )
    {
      this->game_status = cGame::STATUS_ERROR_GAME;
      continue;
    }
    if( action == "UPDATE_MAP" )
    {
      this->remaining_time = atoi( inst.get_param(1).c_str() );
      this->p1->set_position( atoi( inst.get_param(2).c_str() ), atoi( inst.get_param(3).c_str() ) );
      this->p1->set_image( atoi( inst.get_param(4).c_str() ) );
      this->p1->lives = atoi( inst.get_param(5).c_str() );
      this->p1->wins = atoi( inst.get_param(6).c_str() );
      this->p2->set_position( atoi( inst.get_param(7).c_str() ), atoi( inst.get_param(8).c_str() ) );
      this->p2->set_image( atoi( inst.get_param(9).c_str() ) );
      this->p2->lives = atoi( inst.get_param(10).c_str() );
      this->p2->wins = atoi( inst.get_param(11).c_str() );
      int i = 12;
      map<int,cBarrel*>::iterator p = this->obj_barrels.begin();
      while( p != this->obj_barrels.end() )
      {
        cBarrel* map_object = p->second;
        map_object->set_x( atoi( inst.get_param(i).c_str() ) );
        i++;
        p++;
      }
      continue;
    }
  }
  */
}

/******************************************************************************
* Renderiza todo lo de la pantalla inicial (PLAY)
*******************************************************************************/
void cGame::render_init()
{
  // background
  SDL_BlitSurface( this->back_init, 0, this->screen, 0 );
  // start button
  this->dstrect.x = 240;
  this->dstrect.y = 360;
  //SDL_BlitSurface( this->img_play, 0, this->screen, &this->dstrect );
}

/******************************************************************************
* Renderiza la pantalla de espera
*******************************************************************************/
void cGame::render_waiting()
{
  /*
  // background
  SDL_BlitSurface( this->back_init, 0, this->screen, 0 );
  // wait image
  this->dstrect.x = 182;
  this->dstrect.y = 368;
  SDL_BlitSurface( this->img_wait, 0, this->screen, &this->dstrect );
  // depende del estado del server...
  switch( this->tournament_conn.get_status() )
  {
    case cServerConn::NOT_CONNECTED:
    {
      //conectandose...
      this->writer.printf( 250, 370, "Conectandose...", 22, 0xFF6401 );
      break;
    }
    case cServerConn::NO_HOST:
    {
      // servidor caido...
      this->writer.printf( 208, 370, "Servidor no disponible", 22, 0xFF6401 );
      break;
    }
    case cServerConn::CONNECTED:
    {
      // esperando otros jugadores...
      this->writer.printf( 190, 370, "Esperando jugadores...", 22, 0xFF6401 );
      this->writer.printf( 445, 362, this->wait_time, 32, 0xDE2A00, cTextFactory::CENTER );
      break;
    }
    case cServerConn::DISCONNECTED:
    {
      // habia jugadores empares, fuiste desconectado
      this->writer.printf( 196, 370, "Bye! (jugadores impares)", 22, 0xFF6401 );
      this->tournament_conn.disconnect();
      break;
    }
  }
  */
}

/******************************************************************************
* Renderiza mientras se esta jugando
*******************************************************************************/
void cGame::render_playing()
{
  // Fondo
  SDL_BlitSurface( this->back_level, 0, this->screen, 0 );

  // Vidas
  if( this->p1->lives > 0 )
  {
    for( int i = 0; i < this->p1->lives; i++ )
    {
      this->dstrect.x = 55 + 12 * i;
      this->dstrect.y = 19;
      SDL_BlitSurface( this->img_life, 0, this->screen, &this->dstrect );
    }
  }
  if( this->p2->lives > 0 )
  {
    for( int i = 0; i < this->p2->lives; i++ )
    {
      this->dstrect.x = this->w - 155 + 55 + 12 * i;
      this->dstrect.y = 19;
      SDL_BlitSurface( this->img_life, 0, this->screen, &this->dstrect );
    }
  }

  // Paulines
  if( this->p1->wins > 0 )
  {
    for( int i = 0; i < this->p1->wins; i++ )
    {
      this->dstrect.x = 79 + 12 * i;
      this->dstrect.y = 30;
      SDL_BlitSurface( this->img_pauline, 0, this->screen, &this->dstrect );
    }
  }
  if( this->p2->wins > 0 )
  {
    for( int i = 0; i < this->p2->wins; i++ )
    {
      this->dstrect.x = this->w - 155 + 79 + 12 * i;
      this->dstrect.y = 30;
      SDL_BlitSurface( this->img_pauline, 0, this->screen, &this->dstrect );
    }
  }

  // Textos
  this->writer_arcade.printf( 5, 5, "Player 1: " + this->p1->name, 8, 0xFFFFFF );
  this->writer_arcade.printf( 5, 17, "Vidas:", 8, 0xFFFFFF );
  this->writer_arcade.printf( 5, 29, "Rescates:", 8, 0xFFFFFF );
  this->writer_arcade.printf( this->w - 150, 5, "Player 2: " + this->p2->name, 8, 0xFFFFFF );
  this->writer_arcade.printf( this->w - 150, 17, "Vidas:", 8, 0xFFFFFF );
  this->writer_arcade.printf( this->w - 150, 29, "Rescates:", 8, 0xFFFFFF );

  // Pauline
  this->pauline->render( this->screen );

  // Donkey Kongs
  map<int,cDonkeyKong*>::iterator p = this->obj_donkeykongs.begin();
  while( p != this->obj_donkeykongs.end() )
  {
    cDonkeyKong* donkeykong = p->second;
    donkeykong->render( this->screen );
    p++;
  }

  // Barriles de aceite
  map<int,cOil*>::iterator q = this->obj_oils.begin();
  while( q != this->obj_oils.end() )
  {
    cOil* oil = q->second;
    oil->render( this->screen );
    q++;
  }

  // Players
  this->p1->render( this->screen );
  this->p2->render( this->screen );

  // si el resultado de la partida ya esta definido
  if( this->result_end_game != cGame::RES_UNDEFINED )
  {
    // oscurece el fondo
    SDL_BlitSurface( this->back_black, 0, this->screen, 0 );
    SDL_BlitSurface( this->back_black, 0, this->screen, 0 );

    switch( this->result_end_game )
    {
      case cGame::RES_WIN:
      {
        this->writer.printf( this->w / 2, ( this->h / 2 ) - 55, "Felicitaciones...", 22, 0xFFFFFF, cTextFactory::CENTER );
        this->writer.printf( this->w / 2, ( this->h / 2 ) - 50, "Ganaste", 72, 0x84F54D, cTextFactory::CENTER );
        this->writer.printf( this->w / 2, ( this->h / 2 ) + 20, "Eso es, continua asi!", 22, 0xFFFFFF, cTextFactory::CENTER );
        break;
      }
      case cGame::RES_LOSE:
      {
        this->writer.printf( this->w / 2, ( this->h / 2 ) - 55, "Que pena...", 22, 0xFFFFFF, cTextFactory::CENTER );
        this->writer.printf( this->w / 2, ( this->h / 2 ) - 50, "Perdiste", 72, 0xC00000, cTextFactory::CENTER );
        this->writer.printf( this->w / 2, ( this->h / 2 ) + 20, "Mejor suerte para la proxima!", 22, 0xFFFFFF, cTextFactory::CENTER );
        break;
      }
      case cGame::RES_TIE:
      {
        this->writer.printf( this->w / 2, ( this->h / 2 ) - 55, "Que pena...", 22, 0xFFFFFF, cTextFactory::CENTER );
        this->writer.printf( this->w / 2, ( this->h / 2 ) - 50, "Empataste", 72, 0xC00000, cTextFactory::CENTER );
        this->writer.printf( this->w / 2, ( this->h / 2 ) + 20, "Asi no rinde!", 22, 0xFFFFFF, cTextFactory::CENTER );
        break;
      }
    }
  }
}

/******************************************************************************
* Renderiza los resultados parciales (antes de comenzar a jugar una partida y al terminar)
*******************************************************************************/
void cGame::render_results( bool final_results )
{
  /*
  // background
  SDL_BlitSurface( this->back_stats, 0, this->screen, 0 );

  // estadisticas
  this->writer.printf( this->w / 2, 80, "D-Frogger Stats", 28, 0x00C000, cTextFactory::CENTER );
  this->writer.printf( 30, 120, "Nombre", 22, 0xFFFFFF );
  this->writer.printf( 220, 120, "Wins", 22, 0xFFFFFF );
  this->writer.printf( 320, 120, "Losses", 22, 0xFFFFFF );
  this->writer.printf( 445, 120, "Estado", 22, 0xFFFFFF );
  this->writer.printf( 25, 135, "-----------------------------------------------------------------------------------", 22, 0xFFFFFF );
  unsigned int i;
  for( i = 0; i < this->stats.size(); i++ )
  {
    this->writer.printf( 30, 150+22*i, this->stats[i].name, 22, 0xFFFFFF );
    this->writer.printf( 220, 150+22*i, this->stats[i].wins, 22, 0xFFFFFF );
    this->writer.printf( 320, 150+22*i, this->stats[i].losses, 22, 0xFFFFFF );
    this->writer.printf( 445, 150+22*i, this->stats[i].status, 22, 0xFFFFFF );
  }
  int padding = 22 * i;
  if( !final_results )
  {
    this->writer.printf( this->w / 2, 170+padding, "Tu proxima partida es contra", 22, 0xFFFFFF, cTextFactory::CENTER );
    this->writer.printf( this->w / 2, 185+padding, this->opponent_name, 32, 0x84F54D, cTextFactory::CENTER );
    this->writer.printf( this->w / 2, 230+padding, "Comienza en", 22, 0xFFFFFF, cTextFactory::CENTER );
    this->writer.printf( this->w / 2, 240+padding, this->wait_time, 48, 0xFF0000, cTextFactory::CENTER );
  }
  else
  {
    this->writer.printf( this->w / 2, 180+padding, "Estos son los resultados finales", 22, 0xFFFFFF, cTextFactory::CENTER );
    this->writer.printf( this->w / 2, 195+padding, "Felicitaciones por competir!", 32, 0x84F54D, cTextFactory::CENTER );
  }
  */
}

/******************************************************************************
* Renderiza la pantalla de error
*******************************************************************************/
void cGame::render_error()
{
  /*
  // background
  SDL_BlitSurface( this->back_stats, 0, this->screen, 0 );

  // texto
  this->writer.printf( this->w / 2, 130, "Oops!", 72, 0X008000, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 210, "Ocurrio un error al intentar", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 230, "conectarse al servidor de partida.", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 250, "Reinicia el torneo e intenta nuevamente.", 24, 0xFFFFFF, cTextFactory::CENTER );
  */
}

/******************************************************************************
* Renderiza la pantalla de error cuando el torneo se cayo
*******************************************************************************/
void cGame::render_error_game()
{
  /*
  // background
  SDL_BlitSurface( this->back_stats, 0, this->screen, 0 );

  // texto
  this->writer.printf( this->w / 2, 130, "Oops!", 72, 0X008000, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 210, "El servidor de juego no responde", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 230, "y parece haberse caido. Aguarde, esta", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 250, "partida se jugara en otro momento.", 24, 0xFFFFFF, cTextFactory::CENTER );
  */
}


/******************************************************************************
* Renderiza la pantalla de error cuando el torneo se cayo
*******************************************************************************/
void cGame::render_error_tournament()
{
  /*
  // background
  SDL_BlitSurface( this->back_stats, 0, this->screen, 0 );

  // texto
  this->writer.printf( this->w / 2, 130, "Oops!", 72, 0X008000, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 210, "El servidor de torneo no responde", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 230, "y parece haberse caido. Reinicia", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 250, "el torneo e intenta nuevamente.", 24, 0xFFFFFF, cTextFactory::CENTER );
  */
}

/******************************************************************************
* Renderiza la pantalla de confirmacion de salida
*******************************************************************************/
void cGame::render_exit()
{
  // background
  SDL_BlitSurface( this->back_black, 0, this->screen, 0 );
  SDL_BlitSurface( this->back_black, 0, this->screen, 0 );
  SDL_BlitSurface( this->back_black, 0, this->screen, 0 );
  SDL_BlitSurface( this->back_black, 0, this->screen, 0 );

  // texto
  this->writer.printf( this->w / 2, 140, "Salir", 72, 0X008000, cTextFactory::CENTER );
  this->writer.printf( this->w / 2, 220, "Estas seguro que deseas abandonar el juego?", 24, 0xFFFFFF, cTextFactory::CENTER );
  this->writer.printf( this->w / 3, 270, "No", 24, 0x000000, cTextFactory::CENTER, true );
  this->writer.printf( this->w / 3 * 2, 270, "Si!", 24, 0x000000, cTextFactory::CENTER, true );
}

int cGame::get_sdl_key( string key )
{
  if( key == "SDLK_BACKSPACE" ) return SDLK_BACKSPACE;
  if( key == "SDLK_TAB" ) return SDLK_TAB;
  if( key == "SDLK_CLEAR" ) return SDLK_CLEAR;
  if( key == "SDLK_RETURN" ) return SDLK_RETURN;
  if( key == "SDLK_PAUSE" ) return SDLK_PAUSE;
  if( key == "SDLK_ESCAPE" ) return SDLK_ESCAPE;
  if( key == "SDLK_SPACE" ) return SDLK_SPACE;
  if( key == "SDLK_EXCLAIM" ) return SDLK_EXCLAIM;
  if( key == "SDLK_QUOTEDBL" ) return SDLK_QUOTEDBL;
  if( key == "SDLK_HASH" ) return SDLK_HASH;
  if( key == "SDLK_DOLLAR" ) return SDLK_DOLLAR;
  if( key == "SDLK_AMPERSAND" ) return SDLK_AMPERSAND;
  if( key == "SDLK_QUOTE" ) return SDLK_QUOTE;
  if( key == "SDLK_LEFTPAREN" ) return SDLK_LEFTPAREN;
  if( key == "SDLK_RIGHTPAREN" ) return SDLK_RIGHTPAREN;
  if( key == "SDLK_ASTERISK" ) return SDLK_ASTERISK;
  if( key == "SDLK_PLUS" ) return SDLK_PLUS;
  if( key == "SDLK_COMMA" ) return SDLK_COMMA;
  if( key == "SDLK_MINUS" ) return SDLK_MINUS;
  if( key == "SDLK_PERIOD" ) return SDLK_PERIOD;
  if( key == "SDLK_SLASH" ) return SDLK_SLASH;
  if( key == "SDLK_0" ) return SDLK_0;
  if( key == "SDLK_1" ) return SDLK_1;
  if( key == "SDLK_2" ) return SDLK_2;
  if( key == "SDLK_3" ) return SDLK_3;
  if( key == "SDLK_4" ) return SDLK_4;
  if( key == "SDLK_5" ) return SDLK_5;
  if( key == "SDLK_6" ) return SDLK_6;
  if( key == "SDLK_7" ) return SDLK_7;
  if( key == "SDLK_8" ) return SDLK_8;
  if( key == "SDLK_9" ) return SDLK_9;
  if( key == "SDLK_COLON" ) return SDLK_COLON;
  if( key == "SDLK_SEMICOLON" ) return SDLK_SEMICOLON;
  if( key == "SDLK_LESS" ) return SDLK_LESS;
  if( key == "SDLK_EQUALS" ) return SDLK_EQUALS;
  if( key == "SDLK_GREATER" ) return SDLK_GREATER;
  if( key == "SDLK_QUESTION" ) return SDLK_QUESTION;
  if( key == "SDLK_AT" ) return SDLK_AT;
  if( key == "SDLK_LEFTBRACKET" ) return SDLK_LEFTBRACKET;
  if( key == "SDLK_BACKSLASH" ) return SDLK_BACKSLASH;
  if( key == "SDLK_RIGHTBRACKET" ) return SDLK_RIGHTBRACKET;
  if( key == "SDLK_CARET" ) return SDLK_CARET;
  if( key == "SDLK_UNDERSCORE" ) return SDLK_UNDERSCORE;
  if( key == "SDLK_BACKQUOTE" ) return SDLK_BACKQUOTE;
  if( key == "SDLK_a" ) return SDLK_a;
  if( key == "SDLK_b" ) return SDLK_b;
  if( key == "SDLK_c" ) return SDLK_c;
  if( key == "SDLK_d" ) return SDLK_d;
  if( key == "SDLK_e" ) return SDLK_e;
  if( key == "SDLK_f" ) return SDLK_f;
  if( key == "SDLK_g" ) return SDLK_g;
  if( key == "SDLK_h" ) return SDLK_h;
  if( key == "SDLK_i" ) return SDLK_i;
  if( key == "SDLK_j" ) return SDLK_j;
  if( key == "SDLK_k" ) return SDLK_k;
  if( key == "SDLK_l" ) return SDLK_l;
  if( key == "SDLK_m" ) return SDLK_m;
  if( key == "SDLK_n" ) return SDLK_n;
  if( key == "SDLK_o" ) return SDLK_o;
  if( key == "SDLK_p" ) return SDLK_p;
  if( key == "SDLK_q" ) return SDLK_q;
  if( key == "SDLK_r" ) return SDLK_r;
  if( key == "SDLK_s" ) return SDLK_s;
  if( key == "SDLK_t" ) return SDLK_t;
  if( key == "SDLK_u" ) return SDLK_u;
  if( key == "SDLK_v" ) return SDLK_v;
  if( key == "SDLK_w" ) return SDLK_w;
  if( key == "SDLK_x" ) return SDLK_x;
  if( key == "SDLK_y" ) return SDLK_y;
  if( key == "SDLK_z" ) return SDLK_z;
  if( key == "SDLK_DELETE" ) return SDLK_DELETE;
  if( key == "SDLK_KP0" ) return SDLK_KP0;
  if( key == "SDLK_KP1" ) return SDLK_KP1;
  if( key == "SDLK_KP2" ) return SDLK_KP2;
  if( key == "SDLK_KP3" ) return SDLK_KP3;
  if( key == "SDLK_KP4" ) return SDLK_KP4;
  if( key == "SDLK_KP5" ) return SDLK_KP5;
  if( key == "SDLK_KP6" ) return SDLK_KP6;
  if( key == "SDLK_KP7" ) return SDLK_KP7;
  if( key == "SDLK_KP8" ) return SDLK_KP8;
  if( key == "SDLK_KP9" ) return SDLK_KP9;
  if( key == "SDLK_KP_PERIOD" ) return SDLK_KP_PERIOD;
  if( key == "SDLK_KP_DIVIDE" ) return SDLK_KP_DIVIDE;
  if( key == "SDLK_KP_MULTIPLY" ) return SDLK_KP_MULTIPLY;
  if( key == "SDLK_KP_MINUS" ) return SDLK_KP_MINUS;
  if( key == "SDLK_KP_PLUS" ) return SDLK_KP_PLUS;
  if( key == "SDLK_KP_ENTER" ) return SDLK_KP_ENTER;
  if( key == "SDLK_KP_EQUALS" ) return SDLK_KP_EQUALS;
  if( key == "SDLK_UP" ) return SDLK_UP;
  if( key == "SDLK_DOWN" ) return SDLK_DOWN;
  if( key == "SDLK_RIGHT" ) return SDLK_RIGHT;
  if( key == "SDLK_LEFT" ) return SDLK_LEFT;
  if( key == "SDLK_INSERT" ) return SDLK_INSERT;
  if( key == "SDLK_HOME" ) return SDLK_HOME;
  if( key == "SDLK_END" ) return SDLK_END;
  if( key == "SDLK_PAGEUP" ) return SDLK_PAGEUP;
  if( key == "SDLK_PAGEDOWN" ) return SDLK_PAGEDOWN;
  if( key == "SDLK_F1" ) return SDLK_F1;
  if( key == "SDLK_F2" ) return SDLK_F2;
  if( key == "SDLK_F3" ) return SDLK_F3;
  if( key == "SDLK_F4" ) return SDLK_F4;
  if( key == "SDLK_F5" ) return SDLK_F5;
  if( key == "SDLK_F6" ) return SDLK_F6;
  if( key == "SDLK_F7" ) return SDLK_F7;
  if( key == "SDLK_F8" ) return SDLK_F8;
  if( key == "SDLK_F9" ) return SDLK_F9;
  if( key == "SDLK_F10" ) return SDLK_F10;
  if( key == "SDLK_F11" ) return SDLK_F11;
  if( key == "SDLK_F12" ) return SDLK_F12;
  if( key == "SDLK_F13" ) return SDLK_F13;
  if( key == "SDLK_F14" ) return SDLK_F14;
  if( key == "SDLK_F15" ) return SDLK_F15;
  if( key == "SDLK_NUMLOCK" ) return SDLK_NUMLOCK;
  if( key == "SDLK_CAPSLOCK" ) return SDLK_CAPSLOCK;
  if( key == "SDLK_SCROLLOCK" ) return SDLK_SCROLLOCK;
  if( key == "SDLK_RSHIFT" ) return SDLK_RSHIFT;
  if( key == "SDLK_LSHIFT" ) return SDLK_LSHIFT;
  if( key == "SDLK_RCTRL" ) return SDLK_RCTRL;
  if( key == "SDLK_LCTRL" ) return SDLK_LCTRL;
  if( key == "SDLK_RALT" ) return SDLK_RALT;
  if( key == "SDLK_LALT" ) return SDLK_LALT;
  if( key == "SDLK_RMETA" ) return SDLK_RMETA;
  if( key == "SDLK_LMETA" ) return SDLK_LMETA;
  if( key == "SDLK_LSUPER" ) return SDLK_LSUPER;
  if( key == "SDLK_RSUPER" ) return SDLK_RSUPER;
  if( key == "SDLK_MODE" ) return SDLK_MODE;
  if( key == "SDLK_HELP" ) return SDLK_HELP;
  if( key == "SDLK_PRINT" ) return SDLK_PRINT;
  if( key == "SDLK_SYSREQ" ) return SDLK_SYSREQ;
  if( key == "SDLK_BREAK" ) return SDLK_BREAK;
  if( key == "SDLK_MENU" ) return SDLK_MENU;
  if( key == "SDLK_POWER" ) return SDLK_POWER;
  if( key == "SDLK_EURO" ) return SDLK_EURO;
  return 0;
}


/******************************************************************************
* Libera todos los recursos pedidos
*******************************************************************************/
cGame::~cGame()
{
  /*
  SDL_FreeSurface( this->back_init );
  SDL_FreeSurface( this->back_game );
  SDL_FreeSurface( this->back_stats );
  SDL_FreeSurface( this->back_black );
  SDL_FreeSurface( this->img_play );
  SDL_FreeSurface( this->img_wait );
  SDL_FreeSurface( this->frog_life );
  SDL_FreeSurface( this->frog_wins );

  // libera los recursos pedidos en los objetos
  map<int,cBarrel*>::iterator p = this->obj_barrels.begin();
  while( p != this->obj_barrels.end() )
  {
    free( p->second );
    p++;
  }

  // libera la conexion con el juego
  free( this->game_conn );

  // libera los recursos pedidos en los players
  free( this->p1 );
  free( this->p2 );
  */

  if( TTF_WasInit() ) TTF_Quit();
  SDL_Quit();
}

/******************************************************************************
* Envia las posiciones iniciales de los objetos
*******************************************************************************/
std::string cGame::send_new_game()
{
	this->max_lives = 5;		//Es HardCode ya que el server se lo manda
	this->cant_paulines = 2;	//

	string msg = "";
	msg+= "NEW_GAME ";
	msg+= to_str( this->p1->name ) + ";";
	msg+= to_str( this->p2->name ) + ";";
	msg+= to_str( this->max_lives ) + ";";
	msg+= to_str( this->cant_paulines ) + ";";
  msg+= to_str( this->difficulty ) + ";";

	map<int,cDonkeyKong*>::iterator p = this->obj_donkeykongs.begin();
	  while( p != this->obj_donkeykongs.end() )
	  {
		cDonkeyKong* donkeykong = p->second;
		msg+= to_str(donkeykong->x) + ";";
		msg+= to_str(donkeykong->y) + ";";
		msg+= to_str(donkeykong->direction) + ";";

		p++;
	  }

	map<int,cOil*>::iterator po = this->obj_oils.begin();
	  while( po != this->obj_oils.end() )
	  {
		cOil* oil = po->second;
		msg+= to_str(oil->x) + ";";
		msg+= to_str(oil->y) + ";";
		msg+= to_str(oil->direction) + ";";

		po++;
	  }

	msg+= to_str(this->pauline->x) + ";";
	msg+= to_str(this->pauline->y) + "|";
  return msg;
    //printf("%s\n",msg.c_str()); //Hacer send(msg) en vez de printf
}

/******************************************************************************
* Envia las posiciones de todos los objetos del mapa
*******************************************************************************/
std::string cGame::send_update_map()
{

  string msg = "";
  msg+= "UPDATE_MAP ";
  msg+= to_str( this->p1->x ) + ";";
  msg+= to_str( this->p1->y ) + ";";
  msg+= to_str( this->p1->status ) + ";";
  msg+= to_str( this->p1->doing ) + ";";
  msg+= to_str( this->p1->direction ) + ";";
  msg+= to_str( this->p2->x ) + ";";
  msg+= to_str( this->p2->y ) + ";";
  msg+= to_str( this->p2->status ) + ";";
  msg+= to_str( this->p2->doing ) + ";";
  msg+= to_str( this->p2->direction );

  map<int,cDonkeyKong*>::iterator pd = this->obj_donkeykongs.begin();
  while( pd != this->obj_donkeykongs.end() )
  {
	  cDonkeyKong* donkeykong = pd->second;

	  list<cBarrel*>::iterator pb = donkeykong->obj_barrels.begin();
	  while( pb != donkeykong->obj_barrels.end() )
	  {
		cBarrel* barrel = *pb;
		if( barrel->status == cBarrel::STATUS_ACTIVE )
		{
			msg+= ";B";
			msg+= "," + to_str(barrel->x);
			msg+= "," + to_str(barrel->y);
			msg+= "," + to_str(barrel->doing);
		 }
		++pb;
	   }
	  pd++;
  }

  map<int,cOil*>::iterator po = this->obj_oils.begin();
  while( po != this->obj_oils.end() )
  {
	  cOil* oil = po->second;

	  list<cFlame*>::iterator pf = oil->obj_flames.begin();
	  while( pf != oil->obj_flames.end() )
	  {
		cFlame* flame = *pf;
		if( flame->status == cFlame::STATUS_ACTIVE )
		{
			msg+= ";F";
			msg+= "," + to_str(flame->x);
			msg+= "," + to_str(flame->y);
		 }
		 ++pf;
	   }
	  po++;
  }

  msg+= "|";
  return msg;
  //printf("%s\n",msg.c_str()); //Hacer send(msg) en vez de printf
}
