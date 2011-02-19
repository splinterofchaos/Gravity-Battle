 
#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <string>
#include <vector>

void gen_texture( GLuint handle, SDL_Surface* surface );

class Texture
{
    typedef std::string Key;

    struct Item
    {
        Key key; // Can be filename.
        GLuint glHandle;
        unsigned int refCount;

        Item( const Key& key, GLuint handle, unsigned int count )
            : key( key ), glHandle( handle ), refCount( count )
        {
        }
    };

    void reset();

    typedef std::vector< Item > Registry;
    static Registry registery;

    typedef Registry::size_type Ref;

    Ref get_ref();

    Key key;

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

    bool load( const std::string& filename );

    GLuint handle();
};
