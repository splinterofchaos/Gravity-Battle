 
#include "Texture.h"

void gen_texture( GLuint handle, SDL_Surface* surface )
{
    glBindTexture( GL_TEXTURE_2D, handle );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, 
                  GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels );
}

Texture::Registry Texture::registery;

Texture::Ref Texture::get_ref()
{
    for( size_t i=0; i < registery.size(); i++ )
        if( registery[i].key == key )
            return i;
    return -1;
}

Texture::Texture()
{
    //state = IMAGE_NOT_LOADED;
}

Texture::Texture( const std::string& filename )
{
    key = filename;

    Ref ref = get_ref();

    if( ref != -1u ) 
        registery[ ref ].refCount++;
    else 
        load( filename );
}

bool Texture::load( const std::string& filaname )
{
    Ref ref; // Reference to the Item we'll be working on.

    // We only need to reset if we're referencing a texture.
    if( key.size() ) {
        reset();
    }

    key = filaname;

    ref = get_ref();

    // If this Item already exists, and is loaded, we're all set.
    if( get_ref() != -1u && registery[ref].refCount++ != 0 )
        return true;

    // Otherwise, make it.
    registery.push_back( Item(key,0,1) );
    ref = registery.size() - 1;

    glGenTextures( 1, &registery[ref].glHandle );

    // Use SDL to lead the image for simplicity.
    SDL_Surface* sdlSurface = SDL_LoadBMP( filaname.c_str() ); 
    
    if( sdlSurface ) 
    { 
        gen_texture( registery[ref].glHandle, sdlSurface );

        SDL_FreeSurface( sdlSurface );

        return true;
    } 

    return false;
}

void Texture::reset()
{
    Ref ref = get_ref();
    if( ref != -1u && --registery[ ref ].refCount <= 0 ) {
        glDeleteTextures( 1, &registery[ ref ].glHandle );
    }
}

Texture::~Texture()
{
    reset();
}

GLuint Texture::handle()
{
    Ref ref = get_ref();

    if( ref != -1u )
        return registery[ ref ].glHandle; 
    else
        return 0;
}
