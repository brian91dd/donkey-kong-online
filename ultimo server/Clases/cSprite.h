#ifndef cSpriteH
#define cSpriteH

#include <string>
#include <map>
#include <SDL/SDL_image.h>

typedef struct tFrame {
  SDL_Surface* image;
  int time;
} tFrame;

class cSprite
{
  public:
    int w, h;
    cSprite();
    ~cSprite();
    void add_image( std::string image, int time = 100 );
    void animate( bool loop = true );
    void restart_animation();
    SDL_Surface* get_image();

  private:
    unsigned int current_frame;
    unsigned long old_tick;
    std::map<int,tFrame> frames;
};

#endif
