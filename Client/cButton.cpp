#include "cButton.h"

using namespace std;

bool cButton::is_clicked( SDL_Event event )
{
  if( event.button.x > this->size.x && event.button.x < this->size.x + this->size.w )
  {
    if( event.button.y > this->size.y && event.button.y < this->size.y + this->size.h )
    {
      return true;
    }
  }
  return false;
}

void cButton::set_size( SDL_Rect size )
{
  this->size = size;
}
