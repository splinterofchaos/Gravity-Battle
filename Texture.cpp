
#include "Texture.h"

Texture::Texture()
{
    //state = IMAGE_NOT_LOADED;
}

Texture::Texture( const std::string& filename )
{
    load( filename );
}

void Texture::load( const std::string& filaname )
{
    glGenTextures( 1, &glHandle );

    // Use SDL to lead the image for simplicity.
    SDL_Surface *sdlSurface = SDL_LoadBMP( filaname.c_str() ); 
    
    if( sdlSurface ) 
    { 
        glBindTexture( GL_TEXTURE_2D, glHandle );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        
        glTexImage2D( GL_TEXTURE_2D, 0, 4, sdlSurface->w, sdlSurface->h, 0, 
                      GL_BGRA, GL_UNSIGNED_BYTE, sdlSurface->pixels );

        SDL_FreeSurface( sdlSurface );

        // state = NO_ERROR;
    } 
    else 
    {
        // state = IMAGE_NOT_LOADED;
    }

    return; // state;
}

Texture::~Texture()
{
    glDeleteTextures( 1, &glHandle );
}

GLuint Texture::handle()
{
    return glHandle; 
}
