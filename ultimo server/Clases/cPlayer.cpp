#include "cPlayer.h"

using namespace std;

/******************************************************************************
* Crea el objeto player
*******************************************************************************/
cPlayer::cPlayer( char player, string name, int x, int y ) : player_number(player), name(name), wins(0), lives(3), x_start(x), y_start(y), speed_y(0), player(NULL), god_mode_tick(0)
{
  // Cargo las imagenes
  this->player_standby_right.add_image( "images/player_standby_right.png" );
  this->player_standby_left.add_image( "images/player_standby_left.png" );
  this->player_walk_right.add_image( "images/player_walk_right_1.png" );
  this->player_walk_right.add_image( "images/player_walk_right_2.png" );
  this->player_walk_left.add_image( "images/player_walk_left_1.png" );
  this->player_walk_left.add_image( "images/player_walk_left_2.png" );
  this->player_jump_right.add_image( "images/player_jump_right.png" );
  this->player_jump_left.add_image( "images/player_jump_left.png" );
  this->player_up_down.add_image( "images/player_up_down_1.png" );
  this->player_up_down.add_image( "images/player_up_down_2.png" );
  this->player_dead.add_image( "images/player_dead_1.png", 500 );
  this->player_dead.add_image( "images/player_dead_2.png", 500 );
  this->player_dead.add_image( "images/player_dead_3.png", 500 );
  this->player_dead.add_image( "images/player_dead_4.png", 500 );
  this->player_dead.add_image( "images/player_dead_5.png", 500 );
  this->player_win.add_image( "images/player_win.png" );

  // Seteo el ancho y alto (lo tomo de una de las imagenes, pero podria hardcodearlo)
  this->w = this->player_standby_right.w;
  this->h = this->player_standby_right.h;

  // Seteo al jugador en su posicion y estado de inicio
  this->restart();
}

/******************************************************************************
* Seteo al jugador en su posicion y estado de inicio
*******************************************************************************/
void cPlayer::restart()
{
  switch( this->player_number )
  {
    case cPlayer::P1: this->direction = DIR_LEFT; break;
    case cPlayer::P2: this->direction = DIR_RIGHT; break;
  }
  this->set_position( this->x_start, this->y_start );
  this->status = cPlayer::ALIVE;
  this->doing = cPlayer::WAITING;
}

/******************************************************************************
* Actualiza los colores contra los que mapea la posicion en la máscara
*******************************************************************************/
void cPlayer::update_colors( SDL_Surface* mask )
{
  this->colors[0] = get_pixel_color( this->x + this->w/2, this->y, mask ); // HEAD
  this->colors[1] = get_pixel_color( this->x + this->w/2, this->y + this->h, mask ); // FEET
  this->colors[2] = get_pixel_color( this->x, this->y + this->h/2, mask ); // LEFT
  this->colors[3] = get_pixel_color( this->x + this->w, this->y + this->h/2, mask ); // RIGHT
  this->colors[4] = get_pixel_color( this->x + this->w/2, this->y + this->h + 1, mask ); // UNDER
}

/******************************************************************************
* Actualiza la posicion del player en base a las keys presionadas y a la mascara
*******************************************************************************/
void cPlayer::update( map<string,bool> key, SDL_Surface* mask )
{
  switch( this->status )
  {
    case cPlayer::DEAD:
    case cPlayer::ALIVE_WIN:
    {
      if( this->hold_tick + 3500 < SDL_GetTicks() )
      {
        this->god_mode_tick = SDL_GetTicks();
        this->restart();
      }
      break;
    }

    case cPlayer::ALIVE:
    {
      // Actualiza los colores
      this->update_colors( mask );

      // Chequeo si llegó a rescatar a Pauline
      if( this->mask_color( "LEFT", C_WIN ) || this->mask_color( "RIGHT", C_WIN ) )
      {
        this->win();
        return;
      }

      // Si tengo presionado para la derecha y no para la izq, cambio dir a derecha y viceversa
      if( key["RIGHT"] && !key["LEFT"] ) this->direction = DIR_RIGHT;
      else if( !key["RIGHT"] && key["LEFT"] ) this->direction = DIR_LEFT;

      // Si no está sobre el piso y tampoco está subiendo/bajando la escalera
      if( !this->mask_color( "UNDER", C_FLOOR ) && !this->mask_color( "UNDER", C_LADDER_TOP ) && this->doing != cPlayer::CLIMBING )
      {
        // Seteo que esta saltando (a los efectos es lo mismo) y aplico gravedad
        this->doing = cPlayer::JUMPING;
        this->speed_y = this->speed_y + GRAVITATION;

        // Tope de speed
        this->speed_y = this->speed_y > 3 ? 3 : this->speed_y;
      }
      // Está sobre el piso o subiendo/bajando la escalera
      else
      {
        // Fix: Si esta en la escalera y se quedo sin escalera por debajo (escaló pixeles de más)
        if( this->doing == cPlayer::CLIMBING && this->mask_color( "UNDER", C_EMPTY ) ) this->speed_y = 1;
        else
        {
          if( this->doing == cPlayer::JUMPING ) this->doing = cPlayer::WAITING;
          this->speed_y = 0;
        }
      }

      // Si la cabeza esta contra el piso, significa que salto demasiado alto, lo bajo 1px
      while( this->mask_color( "HEAD", C_FLOOR ) )
      {
        this->y = this->y + 1;
        this->speed_y = 0;
        this->update_colors( mask );
      }

      // Actualizo el Y segun la speed en y
      this->y = this->y + this->speed_y;

      // Actualiza los colores
      this->update_colors( mask );

      // Sube escalon
      while( ( this->mask_color( "FEET", C_FLOOR ) || this->mask_color( "FEET", C_LADDER_TOP ) ) && ( this->doing == cPlayer::WAITING || this->doing == cPlayer::WALKING ) )
      {
        this->y = this->y - 1;
        this->speed_y = 0;
        this->update_colors( mask );
      }

      // Acciones que puedo hacer mientras esta parado
      if( this->doing == cPlayer::WAITING )
      {
        if( ( key["RIGHT"] && !key["LEFT"] ) || ( !key["RIGHT"] && key["LEFT"] ) )
          this->doing = cPlayer::WALKING;

        if( key["JUMP"] )
        {
          this->doing = cPlayer::JUMPING;
          this->move_jump();
        }

        if( key["UP"] && !key["DOWN"] && this->mask_color( "FEET", C_LADDER ) )
          this->doing = cPlayer::CLIMBING;

        if( key["DOWN"] && !key["UP"] && this->mask_color( "UNDER", C_LADDER_TOP ) )
          this->doing = cPlayer::CLIMBING;
      }

      // Acciones que puedo hacer mientras esta caminando
      if( this->doing == cPlayer::WALKING )
      {
        if( key["RIGHT"] && !key["LEFT"] ) this->move_right();
        else if( key["LEFT"] && !key["RIGHT"] ) this->move_left();
        else this->doing = cPlayer::WAITING;

        if( key["JUMP"] )
        {
          this->doing = cPlayer::JUMPING;
          this->move_jump();
        }
        if( key["UP"] && !key["DOWN"] && this->mask_color( "FEET", C_LADDER ) )
          this->doing = cPlayer::CLIMBING;

        if( key["DOWN"] && !key["UP"] && this->mask_color( "UNDER", C_LADDER_TOP ) )
          this->doing = cPlayer::CLIMBING;
      }

      // Acciones que puedo hacer mientras esta saltando
      else if( this->doing == cPlayer::JUMPING )
      {
        if( key["RIGHT"] && !key["LEFT"] ) this->move_right();
        else if( key["LEFT"] && !key["RIGHT"] ) this->move_left();
      }

      // Acciones que puedo hacer mientras esta escalando
      else if( this->doing == cPlayer::CLIMBING )
      {
        if( key["UP"] && !key["DOWN"] )
        {
          if( this->mask_color( "FEET", C_LADDER ) ) this->move_climb_up();
          else if( this->mask_color( "FEET", C_LADDER_TOP ) ) { this->doing = cPlayer::WAITING; this->y = this->y - 3; }
        }
        else if( key["DOWN"] && !key["UP"] )
        {
          if( this->mask_color( "UNDER", C_LADDER_TOP ) || this->mask_color( "UNDER", C_LADDER ) ) this->move_climb_down();
          else if( this->mask_color( "UNDER", C_FLOOR ) ) this->doing = cPlayer::WAITING;
        }
      }
      break;
    }
  }
}

/******************************************************************************
* Devuelve true o false si un determinado pixel es del color pasado por parametro
*******************************************************************************/
bool cPlayer::mask_color( string index, int col )
{
  if( index == "HEAD" ) return is_color( this->colors[0], col );
  else if( index == "FEET" ) return is_color( this->colors[1], col );
  else if( index == "LEFT" ) return is_color( this->colors[2], col );
  else if( index == "RIGHT" ) return is_color( this->colors[3], col );
  else if( index == "UNDER" ) return is_color( this->colors[4], col );
  return false;
}

void cPlayer::move_right()
{
  this->x = this->mask_color( "RIGHT", C_FLOOR ) ? this->x : this->x + 2;
}

void cPlayer::move_left()
{
  this->x = this->mask_color( "LEFT", C_FLOOR ) ? this->x : this->x - 2;
}

void cPlayer::move_jump()
{
  this->speed_y = -5.5;
  this->y = this->y - 1;
}

void cPlayer::move_climb_up()
{
  this->y = this->y - 2;
}

void cPlayer::move_climb_down()
{
  this->y = this->y + 2;
}

/******************************************************************************
* Se ejecuta cuando el player muere
*******************************************************************************/
void cPlayer::die()
{
  if( this->god_mode_tick + cPlayer::GOD_MODE < SDL_GetTicks() )
  {
    this->status = cPlayer::DEAD;
    this->hold_tick = SDL_GetTicks();
    this->lives--;
  }
}

/******************************************************************************
* Se ejecuta cuando el player llega a Pauline
*******************************************************************************/
void cPlayer::win()
{
  this->status = cPlayer::ALIVE_WIN;
  this->hold_tick = SDL_GetTicks();
  this->wins++;
}

/******************************************************************************
* Setea la posicion (x,y) pasada por parametro
*******************************************************************************/
void cPlayer::set_position( int x, int y )
{
  this->x = x;
  this->y = y;
}

/******************************************************************************
* Renderiza la rana en pantalla
*******************************************************************************/
void cPlayer::render( SDL_Surface* screen )
{
  SDL_Rect player_pos;
  player_pos.x = this->x;
  player_pos.y = this->y;

  // Pone la imagen que corresponda
  switch( this->status )
  {
    case cPlayer::DEAD:
    {
      if( this->player != &this->player_dead ) this->player_dead.restart_animation();
      this->player = &this->player_dead;
      this->player->animate( false );
      break;
    }

    case cPlayer::ALIVE:
    {
      switch( this->doing )
      {
        case cPlayer::WAITING: this->player = ( this->direction == DIR_RIGHT ) ? &this->player_standby_right : &this->player_standby_left; break;
        case cPlayer::WALKING: this->player = ( this->direction == DIR_RIGHT ) ? &this->player_walk_right : &this->player_walk_left; break;
        case cPlayer::JUMPING: this->player = ( this->direction == DIR_RIGHT ) ? &this->player_jump_right : &this->player_jump_left; break;
        case cPlayer::CLIMBING: this->player = &this->player_up_down; break;
      }
      unsigned int tick_tmp = SDL_GetTicks();
      if( this->god_mode_tick + cPlayer::GOD_MODE > tick_tmp )
      {
        if( ( ( SDL_GetTicks() - this->god_mode_tick ) % 300  ) < 150 )
          this->player = &this->player_invisible;
      }
      this->player->animate();
      break;
    }

    case cPlayer::ALIVE_WIN:
    {
      if( this->player != &this->player_win )
        this->player_win.restart_animation();
      this->player = &this->player_win;
      this->player->animate();
      break;
    }
  }

  SDL_BlitSurface( this->player->get_image(), 0, screen, &player_pos );
}

/******************************************************************************
* Libera recursos
*******************************************************************************/
cPlayer::~cPlayer()
{
}
