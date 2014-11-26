#include "cGame.h"

#include <SDL/SDL.h>
#include <unistd.h>
#include <cmath>

using namespace std;

cGame::cGame( string player_name )
:
result_end_game(cGame::RES_UNDEFINED),p1(NULL),p2(NULL),pauline(NULL),tournament_in_progress(true),
game_status(cGame::STATUS_INIT),running(true),exit_cofirm(false),
tick_time(0),player_name(player_name),p1_name(player_name),max_lives("3"),max_rescues("3"),difficulty(DIF_HARD)
{
  // Inicializa que botones están presionados en false
  this->key["JUMP"] = false;
  this->key["UP"] = false;
  this->key["DOWN"] = false;
  this->key["RIGHT"] = false;
  this->key["LEFT"] = false;

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
  this->screen = SDL_SetVideoMode( 800, 600, 32, SDL_SWSURFACE );
  if ( !this->screen ) printf( "Unable to set 800x600 video: %s\n", SDL_GetError() );

  // Inicializa las fonts TTF
  if( TTF_Init() == -1 ) printf( "Unable to init TTF: %s\n", SDL_GetError() );

  // Carga los fondos que voy a usar
  this->back_init = load_image( "images/back_init.png" );
  this->back_level = load_image( "images/back_level.png" );
  this->back_stats = load_image( "images/back_stats.png" );
  this->back_info = load_image( "images/back_info.png" );
  this->level_mask = load_image( "images/level_mask.png" );
  this->background = this->back_level;

  // Cargo una imagen negra con alpha para un efecto de oscurecimiento copado
  this->back_black = load_image( "images/back_black.png" );
  SDL_SetAlpha( this->back_black, SDL_RLEACCEL|SDL_SRCALPHA, 200 );

  // Carga las imagenes que voy a usar
  this->img_life = load_image( "images/life.png", true );
  this->img_pauline = load_image( "images/pauline.png", true );

  this->h = this->background->h;
  this->w = this->background->w;
  this->writer.init( this->screen, "fonts/motor.ttf" );
  this->writer_off.init( this->screen, "fonts/motor.ttf", 0xCCCCCC, 0x666666, 0x000000 );
  this->writer_arcade.init( this->screen, "fonts/arcade.ttf" );
}

/******************************************************************************
* Crea un nuevo juego
*******************************************************************************/
void cGame::new_game( vector<map_object> objetos, bool multiplayer )
{
  // Hace un free (por si se ejecuto un juego anteriormente)
  this->free_resources();

  // Creo los objetos en el mapa
  int dk_count = 0, oil_count = 0;
  for( vector<map_object>::size_type i = 0; i != objetos.size(); i++ )
  {
    if( objetos[i].type == OBJ_DK ) this->obj_donkeykongs[dk_count++] = new cDonkeyKong( objetos[i].x, objetos[i].y, objetos[i].direction, this->difficulty );
    else if( objetos[i].type == OBJ_OIL ) this->obj_oils[oil_count++] = new cOil( objetos[i].x, objetos[i].y, objetos[i].direction, this->difficulty );
    else if( objetos[i].type == OBJ_PAULINE ) this->pauline = new cPauline( objetos[i].x, objetos[i].y );
  }

  // Creo el player
  this->p1 = new cPlayer( cPlayer::P1, this->p1_name, 349, 536, atoi( this->max_lives.c_str() ) );
  if( multiplayer ) this->p2 = new cPlayer( cPlayer::P2, this->p2_name, 456, 536, atoi( this->max_lives.c_str() ) );

  // Seteo el estado de jugando
  this->game_status = cGame::STATUS_PLAYING;
}

/******************************************************************************
* Carga todos los datos necesarios del mapa y comienza a correr el juego
*******************************************************************************/
void cGame::run()
{
  unsigned int interval = 1000000 / cGame::FPS;

  int frame_number = 0;
  string fps = "-";
  unsigned int fps_start = 0, delay_fixer = 0;

  while( this->running )
  {
    // Toma los tiempos para el calculo de FPS y de correccion de delay
    delay_fixer = SDL_GetTicks();
    if( frame_number == 0 ) fps_start = delay_fixer;

    // Procesa la info que se haya obtenido del teclado
    this->process_keyboard();

    // Procesa la info que haya llegado por sockets
    if( this->multiplayer ) this->process_messages();
    else this->update_playing();

    // Limpia la pantalla
    SDL_FillRect( this->screen, 0, SDL_MapRGB( this->screen->format, 0, 0, 0 ) );

    // Renderiza
    this->render();

    // Hace un pequeño delay para matar a la maquina
    // A interval (tiempo fijo) le resta el tiempo que le llevo el loop
    unsigned int delta = ( SDL_GetTicks() - delay_fixer ) * 1000;
    if( delta < interval ) usleep( interval - delta );

    // Recalcula los FPS cada "cGame::FPS/2" frames
    frame_number++;
    if( frame_number == cGame::FPS / 2 )
    {
      fps = to_str( floor( frame_number / ( (float)( SDL_GetTicks() - fps_start ) ) * 1000 * 10 ) / 10 );
      frame_number = 0;
    }
    this->writer.printf( 7, 575, "FPS: " + fps, 18, 0xFFFFFF );

    // Flip de la pantalla (oculta lo viejo, muestra lo nuevo)
    SDL_Flip( this->screen );
  }

  // Si salgo del juego, aviso al servidor
  if( this->multiplayer ) this->server.send( "EXIT" );

  return;
}

/******************************************************************************
* LOGICA DEL JUEGO
*******************************************************************************/
void cGame::update_playing()
{
  if( this->game_status == cGame::STATUS_PLAYING )
  {
    // Actualizo la posicion del player
    this->p1->update( this->key, this->level_mask );

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
  }
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
void cGame::process_keyboard()
{
  SDL_Event event;

  while( SDL_PollEvent( &event ) )
  {
    if( event.type == SDL_QUIT || ( event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) )
    {
      this->running = false;
      //this->exit_cofirm = true;
      break;
    }

    if( !this->exit_cofirm )
    {
      switch( this->game_status )
      {
        case cGame::STATUS_INIT:
        {
          if( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1 )
          {
            if( btn_play_singleplayer.is_clicked( event ) )
            {
              this->multiplayer = false;
              vector<map_object> objetos;
              map_object obj;
              // DK 1
              obj.type = OBJ_DK;
              obj.x = 332;
              obj.y = 211;
              obj.direction = DIR_LEFT;
              objetos.push_back( obj );
              // DK 2
              obj.type = OBJ_DK;
              obj.x = 443;
              obj.y = 211;
              obj.direction = DIR_RIGHT;
              objetos.push_back( obj );
              // OIL 1
              obj.type = OBJ_OIL;
              obj.x = 392;
              obj.y = 530;
              obj.direction = DIR_LEFT;
              objetos.push_back( obj );
              // OIL 2
              obj.type = OBJ_OIL;
              obj.x = 408;
              obj.y = 530;
              obj.direction = DIR_RIGHT;
              objetos.push_back( obj );
              // PAULINE
              obj.type = OBJ_PAULINE;
              obj.x = 379;
              obj.y = 144;
              objetos.push_back( obj );
              this->new_game( objetos );
            }
            if( btn_play_multiplayer.is_clicked( event ) )
            {
              this->multiplayer = true;
              server.connect( this->ip, this->port );
              server.start( ( void* ) &this->messages );
              server.detach();
              server.send( this->player_name );
            }
            if( btn_exit.is_clicked( event ) )
            {
              this->exit_cofirm = true;
            }
          }
          break;
        }

        case cGame::STATUS_PLAYING:
        {
          if( this->multiplayer )
          {
            if( event.type == SDL_KEYDOWN )
            {
              /*
              if( event.key.keysym.sym == this->key_up ) this->server.send( "KEYDOWN UP" );
              else if( event.key.keysym.sym == this->key_dw ) this->server.send( "KEYDOWN DOWN" );
              else if( event.key.keysym.sym == this->key_rt ) this->server.send( "KEYDOWN RIGHT" );
              else if( event.key.keysym.sym == this->key_lt ) this->server.send( "KEYDOWN LEFT" );
              else if( event.key.keysym.sym == this->key_jump ) this->server.send( "KEYDOWN JUMP" );
              */
              if( event.key.keysym.sym == this->key_up ) this->server.send( "6 4" );
              else if( event.key.keysym.sym == this->key_dw ) this->server.send( "6 3" );
              else if( event.key.keysym.sym == this->key_rt ) this->server.send( "6 2" );
              else if( event.key.keysym.sym == this->key_lt ) this->server.send( "6 1" );
              else if( event.key.keysym.sym == this->key_jump ) this->server.send( "6 5" );
            }
            else if( event.type == SDL_KEYUP )
            {
              /*
              if( event.key.keysym.sym == this->key_up ) this->server.send( "KEYUP UP" );
              else if( event.key.keysym.sym == this->key_dw ) this->server.send( "KEYUP DOWN" );
              else if( event.key.keysym.sym == this->key_rt ) this->server.send( "KEYUP RIGHT" );
              else if( event.key.keysym.sym == this->key_lt ) this->server.send( "KEYUP LEFT" );
              else if( event.key.keysym.sym == this->key_jump ) this->server.send( "KEYUP JUMP" );
              */
              if( event.key.keysym.sym == this->key_up ) this->server.send( "7 4" );
              else if( event.key.keysym.sym == this->key_dw ) this->server.send( "7 3" );
              else if( event.key.keysym.sym == this->key_rt ) this->server.send( "7 2" );
              else if( event.key.keysym.sym == this->key_lt ) this->server.send( "7 1" );
              else if( event.key.keysym.sym == this->key_jump ) this->server.send( "7 5" );
            }
          }
          else
          {
            if( event.type == SDL_KEYDOWN )
            {
              if( event.key.keysym.sym == this->key_up ) this->key["UP"] = true;
              else if( event.key.keysym.sym == this->key_dw ) this->key["DOWN"] = true;
              else if( event.key.keysym.sym == this->key_rt ) this->key["RIGHT"] = true;
              else if( event.key.keysym.sym == this->key_lt ) this->key["LEFT"] = true;
              else if( event.key.keysym.sym == this->key_jump ) this->key["JUMP"] = true;
            }
            else if( event.type == SDL_KEYUP )
            {
              if( event.key.keysym.sym == this->key_up ) this->key["UP"] = false;
              else if( event.key.keysym.sym == this->key_dw ) this->key["DOWN"] = false;
              else if( event.key.keysym.sym == this->key_rt ) this->key["RIGHT"] = false;
              else if( event.key.keysym.sym == this->key_lt ) this->key["LEFT"] = false;
              else if( event.key.keysym.sym == this->key_jump ) this->key["JUMP"] = false;
            }
          }
        }
      }
    }
    else
    {
      if( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1 )
      {
        if( btn_no.is_clicked( event ) )
        {
          this->exit_cofirm = false;
        }
        if( btn_yes.is_clicked( event ) )
        {
          this->running = false;
        }
      }
    }
  }
}

/******************************************************************************
* Procesa la informacion que haya llegado desde los sockets
*******************************************************************************/
void cGame::process_messages()
{
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
      this->server.disconnect();
      continue;
    }
    if( action == "STATS" )
    {
      this->stats.clear();
      for( unsigned int i = 1; i <= inst.size_params(); i++ )
      {
        string player_str = inst.get_param(i);
        vector<string> player_vtr = inst.split( player_str, ',' );
        cPlayerStats player_stats;
        player_stats.name = player_vtr[1];
        player_stats.status = player_vtr[2];
        player_stats.life = player_vtr[3];
        player_stats.paulines = player_vtr[3];
        player_stats.position = player_vtr[4];
        this->stats.push_back( player_stats );
      }
      continue;
    }
    if( action == "NEW_GAME" )
    {
      this->p1_name = inst.get_param(1);
      this->p2_name = inst.get_param(2);
      this->max_lives = inst.get_param(3);
      this->max_rescues = inst.get_param(4);
      this->difficulty = atoi( inst.get_param(5).c_str() );
      vector<map_object> objetos;
      map_object obj;
      // DK 1
      obj.type = OBJ_DK;
      obj.x = atoi( inst.get_param(6).c_str() );
      obj.y = atoi( inst.get_param(7).c_str() );
      obj.direction = atoi( inst.get_param(8).c_str() );
      objetos.push_back( obj );
      // DK 2
      obj.type = OBJ_DK;
      obj.x = atoi( inst.get_param(9).c_str() );
      obj.y = atoi( inst.get_param(10).c_str() );
      obj.direction = atoi( inst.get_param(11).c_str() );
      objetos.push_back( obj );
      // OIL 1
      obj.type = OBJ_OIL;
      obj.x = atoi( inst.get_param(12).c_str() );
      obj.y = atoi( inst.get_param(13).c_str() );
      obj.direction = atoi( inst.get_param(14).c_str() );
      objetos.push_back( obj );
      // OIL 2
      obj.type = OBJ_OIL;
      obj.x = atoi( inst.get_param(15).c_str() );
      obj.y = atoi( inst.get_param(16).c_str() );
      obj.direction = atoi( inst.get_param(17).c_str() );
      objetos.push_back( obj );
      // PAULINE
      obj.type = OBJ_PAULINE;
      obj.x = atoi( inst.get_param(18).c_str() );
      obj.y = atoi( inst.get_param(19).c_str() );
      objetos.push_back( obj );

      this->new_game( objetos, true );
      continue;
    }
    if( action == "UPDATE_LIFE" )
    {
      cPlayer* player = atoi( inst.get_param(1).c_str() ) == cPlayer::P1 ? this->p1 : this->p2;
      player->lives = atoi( inst.get_param(2).c_str() );
      continue;
    }
    if( action == "UPDATE_PAULINE" )
    {
      cPlayer* player = atoi( inst.get_param(1).c_str() ) == cPlayer::P1 ? this->p1 : this->p2;
      player->wins = atoi( inst.get_param(2).c_str() );
      continue;
    }
    if( action == "NEW_BARREL" )
    {
      cDonkeyKong* dk = atoi( inst.get_param(1).c_str() ) == 1 ? this->obj_donkeykongs[0] : this->obj_donkeykongs[1];
      dk->doing = cDonkeyKong::DO_THROWING;
      continue;
    }
    if( action == "TOURNAMENT_FINISHED" )
    {
      this->game_status = cGame::STATUS_RESULTS;
      this->tournament_in_progress = false;
      continue;
    }
    if( action == "YOU_WIN" )
    {
      this->result_end_game = cGame::RES_WIN;
      continue;
    }
    if( action == "YOU_LOSE" )
    {
      this->result_end_game = cGame::RES_LOSE;
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
      this->p1->set_position( atoi( inst.get_param(1).c_str() ), atoi( inst.get_param(2).c_str() ) );
      this->p1->status =atoi( inst.get_param(3).c_str() );
      this->p1->doing = atoi( inst.get_param(4).c_str() );
      this->p1->direction = atoi( inst.get_param(5).c_str() );
      this->p2->set_position( atoi( inst.get_param(6).c_str() ), atoi( inst.get_param(7).c_str() ) );
      this->p2->status =atoi( inst.get_param(8).c_str() );
      this->p2->doing = atoi( inst.get_param(9).c_str() );
      this->p2->direction = atoi( inst.get_param(10).c_str() );

      unsigned int inc_barrel = 0;
      unsigned int inc_flame = 0;
      for( unsigned int i = 11; i <= inst.size_params(); i++ )
      {
        string obj_str = inst.get_param(i);
        vector<string> obj_vtr = inst.split( obj_str, ',' );
        if( obj_vtr[0] == "B" )
        {
          if( inc_barrel >= this->obj_barrels.size() ) this->obj_barrels.push_back( new cBarrel() );
          this->obj_barrels[inc_barrel]->status = cBarrel::STATUS_ACTIVE;
          this->obj_barrels[inc_barrel]->x = atoi( obj_vtr[1].c_str() );
          this->obj_barrels[inc_barrel]->y = atoi( obj_vtr[2].c_str() );
          this->obj_barrels[inc_barrel]->doing = atoi( obj_vtr[3].c_str() );
          inc_barrel++;
        }
        else if( obj_vtr[0] == "F" )
        {
          if( inc_flame >= this->obj_flames.size() ) this->obj_flames.push_back( new cFlame() );
          this->obj_flames[inc_flame]->status = cFlame::STATUS_ACTIVE;
          this->obj_flames[inc_flame]->x = atoi( obj_vtr[1].c_str() );
          this->obj_flames[inc_flame]->y = atoi( obj_vtr[2].c_str() );
          inc_flame++;
        }
      }
      for( unsigned int i = inc_barrel; i < this->obj_barrels.size(); i++ )
        this->obj_barrels[i]->status = cBarrel::STATUS_INACTIVE;
      for( unsigned int i = inc_flame; i < this->obj_flames.size(); i++ )
        this->obj_flames[i]->status = cFlame::STATUS_INACTIVE;
      continue;
    }
  }
}

/******************************************************************************
* Renderiza el juego
*******************************************************************************/
void cGame::render()
{
  switch( this->game_status )
  {
    /************************************************************************************
    * PANTALLA INICIAL (antes de presionar PLAY)
    ************************************************************************************/
    case cGame::STATUS_INIT:
    {
      SDL_BlitSurface( this->back_init, 0, this->screen, 0 );

      switch( this->server.get_status() )
      {
        case cServerConn::NOT_DEFINED:
        case cServerConn::NO_HOST:
        case cServerConn::DISCONNECTED:
        {
          this->btn_play_singleplayer.set_size( this->writer.printf( this->w / 2, 300, "Jugar Singleplayer", 24, 0x000000, cTextFactory::CENTER, true ) );
          this->btn_play_multiplayer.set_size( this->writer.printf( this->w / 2, 350, "Jugar Multiplayer ", 24, 0x000000, cTextFactory::CENTER, true ) );
          this->btn_exit.set_size( this->writer.printf( this->w / 2, 400, "          Salir          ", 24, 0x000000, cTextFactory::CENTER, true ) );
          break;
        }
        case cServerConn::CONNECTED:
        case cServerConn::NOT_CONNECTED:
        {
          this->writer_off.printf( this->w / 2, 300, "Jugar Singleplayer", 24, 0x000000, cTextFactory::CENTER, true );
          this->writer_off.printf( this->w / 2, 350, "Jugar Multiplayer ", 24, 0x000000, cTextFactory::CENTER, true );
          this->btn_exit.set_size( this->writer.printf( this->w / 2, 400, "          Salir          ", 24, 0x000000, cTextFactory::CENTER, true ) );
          break;
        }
      }

      switch( this->server.get_status() )
      {
        case cServerConn::NOT_CONNECTED:
        {
          this->writer_arcade.printf( this->w / 2, 460, "Conectandose al servidor", 8, 0xFFFFFF, cTextFactory::CENTER );
          this->writer_arcade.printf( this->w / 2, 471, "Espere un momento por favor", 8, 0xFFFFFF, cTextFactory::CENTER );
          break;
        }
        case cServerConn::NO_HOST:
        {
          this->writer_arcade.printf( this->w / 2, 460, "Servidor no disponible", 8, 0xFFFFFF, cTextFactory::CENTER );
          this->writer_arcade.printf( this->w / 2, 471, "Reintente en otro momento", 8, 0xFFFFFF, cTextFactory::CENTER );
          break;
        }
        case cServerConn::CONNECTED:
        {
          this->writer_arcade.printf( this->w / 2, 460, "Conectado, esperando jugadores", 8, 0xFFFFFF, cTextFactory::CENTER );
          this->writer_arcade.printf( this->w / 2, 471, "Segundos para comenzar: " + this->wait_time, 8, 0xFFFFFF, cTextFactory::CENTER );
          break;
        }
        case cServerConn::DISCONNECTED:
        {
          this->writer_arcade.printf( this->w / 2, 460, "Has sido desconectado debido a", 8, 0xFFFFFF, cTextFactory::CENTER );
          this->writer_arcade.printf( this->w / 2, 471, "el torneo necesita jugadores pares", 8, 0xFFFFFF, cTextFactory::CENTER );
          this->writer_arcade.printf( this->w / 2, 482, "y tu fuiste el ultimo en anotarse", 8, 0xFFFFFF, cTextFactory::CENTER );
          this->server.disconnect();
          break;
        }
      }
      break;
    }


    /************************************************************************************
    * PANTALLA DEL JUEGO (mientras estamos jugando)
    ************************************************************************************/
    case cGame::STATUS_PLAYING:
    {
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
      if( this->multiplayer && this->p2->lives > 0 )
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
      if( this->multiplayer && this->p2->wins > 0 )
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

      if( this->multiplayer )
      {
        this->writer_arcade.printf( this->w - 150, 5, "Player 2: " + this->p2->name, 8, 0xFFFFFF );
        this->writer_arcade.printf( this->w - 150, 17, "Vidas:", 8, 0xFFFFFF );
        this->writer_arcade.printf( this->w - 150, 29, "Rescates:", 8, 0xFFFFFF );
      }

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

      // Aceite
      map<int,cOil*>::iterator q = this->obj_oils.begin();
      while( q != this->obj_oils.end() )
      {
        cOil* oil = q->second;
        oil->render( this->screen );
        q++;
      }

      // Barriles
      for( unsigned int i = 0; i < this->obj_barrels.size(); i++ )
        this->obj_barrels[i]->render( this->screen );

      // Llamas
      for( unsigned int i = 0; i < this->obj_flames.size(); i++ )
        this->obj_flames[i]->render( this->screen );

      // Players
      this->p1->render( this->screen );
      if( this->multiplayer ) this->p2->render( this->screen );

      break;
    }

    /************************************************************************************
    * PANTALLA DEL RESULTADOS PARCIALES (antes y despues de cada partida)
    ************************************************************************************/
    case cGame::STATUS_RESULTS:
    {
      // background
      SDL_BlitSurface( this->back_stats, 0, this->screen, 0 );

      // estadisticas
      this->writer_arcade.printf( this->w / 2, 95, "Tabla de posiciones", 12, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( 110, 145, "Nombre", 12, 0xFFFFFF );
      this->writer_arcade.printf( 230, 145, "Estado", 12, 0xFFFFFF );
      this->writer_arcade.printf( 400, 145, "Vidas", 12, 0xFFFFFF );
      this->writer_arcade.printf( 485, 145, "Rescates", 12, 0xFFFFFF );
      this->writer_arcade.printf( 595, 145, "Posicion", 12, 0xFFFFFF );
      this->writer_arcade.printf( 108, 160, "-------------------------------------------------", 12, 0xFFFFFF );

      unsigned int i;
      for( i = 0; i < this->stats.size(); i++ )
      {
        this->writer_arcade.printf( 110, 180+22*i, this->stats[i].name, 12, 0xFFFFFF );
        this->writer_arcade.printf( 230, 180+22*i, this->stats[i].status, 12, 0xFFFFFF );
        this->writer_arcade.printf( 400, 180+22*i, this->stats[i].life, 12, 0xFFFFFF );
        this->writer_arcade.printf( 485, 180+22*i, this->stats[i].paulines, 12, 0xFFFFFF );
        this->writer_arcade.printf( 595, 180+22*i, this->stats[i].position, 12, 0xFFFFFF );
      }
      if( this->tournament_in_progress )
      {
        switch( this->result_end_game )
        {
          case cGame::RES_WIN:
          {
            this->writer_arcade.printf( this->w / 2, 465, "Felicitaciones, ganaste!", 12, 0x84F54D, cTextFactory::CENTER );
            this->writer_arcade.printf( this->w / 2, 480, "Estos son los resultados parciales", 12, 0xFFFFFF, cTextFactory::CENTER );
            this->writer_arcade.printf( this->w / 2, 495, "Espera a que empiece tu proximo partido", 12, 0xFFFFFF, cTextFactory::CENTER );
            break;
          }
          case cGame::RES_LOSE:
          {
            this->writer_arcade.printf( this->w / 2, 465, "Que pena, perdiste!", 12, 0xC00000, cTextFactory::CENTER );
            this->writer_arcade.printf( this->w / 2, 480, "Estos son los resultados parciales", 12, 0xFFFFFF, cTextFactory::CENTER );
            this->writer_arcade.printf( this->w / 2, 495, "Espera al final del torneo para ver quien gana!", 12, 0xFFFFFF, cTextFactory::CENTER );
            break;
          }
        }
      }
      else
      {
        this->writer_arcade.printf( this->w / 2, 465, "El torneo ha terminado!", 12, 0xFFFF00, cTextFactory::CENTER );
        this->writer_arcade.printf( this->w / 2, 480, "Estos son los resultados finales", 12, 0xFFFFFF, cTextFactory::CENTER );
        this->writer_arcade.printf( this->w / 2, 495, "Muchas gracias por competir!", 12, 0xFFFFFF, cTextFactory::CENTER );
      }
      break;
    }

    /************************************************************************************
    * PANTALLA DE ERROR (en caso de no poder conectar con el game server)
    ************************************************************************************/
    case cGame::STATUS_ERROR:
    {
      SDL_BlitSurface( this->back_info, 0, this->screen, 0 );
      this->writer_arcade.printf( this->w / 2, 200, "Oops!", 36, 0X008000, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 260, "Ocurrio un error al intentar", 14, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 280, "conectarse al servidor de partida.", 14, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 300, "Reinicia el torneo e intenta nuevamente.", 14, 0xFFFFFF, cTextFactory::CENTER );
      break;
    }

    /************************************************************************************
    * PANTALLA DE ERROR (en caso de no poder conectar con el game server)
    ************************************************************************************/
    case cGame::STATUS_ERROR_TOURNAMENT:
    {
      SDL_BlitSurface( this->back_info, 0, this->screen, 0 );
      this->writer_arcade.printf( this->w / 2, 200, "Oops!", 36, 0X008000, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 260, "El servidor de torneo no responde", 14, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 280, "y parece haberse caido. Reinicia", 14, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 300, "el torneo e intenta nuevamente.", 14, 0xFFFFFF, cTextFactory::CENTER );
      break;
    }

    /************************************************************************************
    * PANTALLA DE ERROR (en caso de no poder conectar con el game server)
    ************************************************************************************/
    case cGame::STATUS_ERROR_GAME:
    {
      SDL_BlitSurface( this->back_info, 0, this->screen, 0 );
      this->writer_arcade.printf( this->w / 2, 200, "Oops!", 36, 0X008000, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 260, "El servidor de juego no responde", 14, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 280, "y parece haberse caido. Aguarde, esta", 14, 0xFFFFFF, cTextFactory::CENTER );
      this->writer_arcade.printf( this->w / 2, 300, "partida se jugara en otro momento.", 14, 0xFFFFFF, cTextFactory::CENTER );
      break;
    }
  }

  // si el flag esta activado, muestro la pantalla de salida
  if( this->exit_cofirm )
  {
    SDL_BlitSurface( this->back_black, 0, this->screen, 0 );
    this->writer.printf( this->w / 2, 140, "Salir", 72, 0X008000, cTextFactory::CENTER );
    this->writer.printf( this->w / 2, 220, "Estas seguro que deseas abandonar el juego?", 24, 0xFFFFFF, cTextFactory::CENTER );
    this->btn_no.set_size( this->writer.printf( this->w / 3, 270, "No", 24, 0x000000, cTextFactory::CENTER, true ) );
    this->btn_yes.set_size( this->writer.printf( this->w / 3 * 2, 270, "Si!", 24, 0x000000, cTextFactory::CENTER, true ) );
  }
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
void cGame::free_resources()
{
  // Pauline
  free( this->pauline );
  this->pauline = NULL;

  // Players
  free( this->p1 );
  free( this->p2 );
  this->p1 = NULL;
  this->p2 = NULL;

  // Donkey Kongs
  map<int,cDonkeyKong*>::iterator p = this->obj_donkeykongs.begin();
  while( p != this->obj_donkeykongs.end() )
  {
    free( p->second );
    p++;
  }

  // Aceites
  map<int,cOil*>::iterator q = this->obj_oils.begin();
  while( q != this->obj_oils.end() )
  {
    free( q->second );
    q++;
  }

  // Barriles
  for( unsigned int i = 0; i < this->obj_barrels.size(); i++ )
    free( this->obj_barrels[i] );

  // Llamas
  for( unsigned int i = 0; i < this->obj_flames.size(); i++ )
    free( this->obj_flames[i] );

  this->tick_time = 0;
  this->result_end_game = cGame::RES_UNDEFINED;
}


/******************************************************************************
* Libera todos los recursos pedidos
*******************************************************************************/
cGame::~cGame()
{
  SDL_FreeSurface( this->back_black );
  SDL_FreeSurface( this->back_level );
  SDL_FreeSurface( this->back_init );
  SDL_FreeSurface( this->back_stats );
  SDL_FreeSurface( this->back_info );
  SDL_FreeSurface( this->level_mask );
  SDL_FreeSurface( this->img_life );
  SDL_FreeSurface( this->img_pauline );

  this->free_resources();

  if( TTF_WasInit() ) TTF_Quit();
  SDL_Quit();
}
