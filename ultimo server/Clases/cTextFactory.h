#ifndef CTEXTFACTORY_H
#define CTEXTFACTORY_H

#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <string>
#include <map>

class cTextFactory
{
  public:
    void init( SDL_Surface* screen, std::string font );
    void printf( int x, int y, std::string text, int size, int color, char align = cTextFactory::LEFT, bool button = false );
    ~cTextFactory();

    static const char LEFT = 1;
    static const char RIGHT = 2;
    static const char CENTER = 3;

  private:
    SDL_Rect pos;
    std::string font;
    SDL_Surface* screen;
    std::map<int,TTF_Font*> fonts;
    std::map<std::string,SDL_Surface*> strings;
    std::map<std::string,int> widths;
    std::map<std::string,int> height;
};

#endif
