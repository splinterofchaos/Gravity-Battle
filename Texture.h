
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#pragma once

#include <string>

class Texture
{
    GLuint glHandle; // Identifies the texture.

  public:
    Texture();
    Texture( const std::string& filename );
    ~Texture();

//    enum LoadError {
//        NO_ERROR,
//        WIDTH_NOT_SIZE_OF_2,
//        HEIGHT_NOT_SIZE_OF_2,
//        IMAGE_NOT_LOADED,
//    }
//    int state;

    void load( const std::string& filename );

    GLuint handle();
};
