
#include "Vector.h"

#include <iterator>
#include <SDL/SDL_opengl.h>

namespace draw {

// opengl_traits code from
// http://www.gamedev.net/reference/programming/features/ogltypetraits/
template< class T > struct opengl_traits {};

template<> struct opengl_traits< unsigned int >
{
    enum { GL_TYPE = GL_UNSIGNED_INT };
};

template<> struct opengl_traits< int >
{
    enum { GL_TYPE = GL_INT };
};

template<> struct opengl_traits< float >
{
    enum { GL_TYPE = GL_FLOAT };
};

// PROTOTYPES
template< typename T >
void draw( T* verts, size_t nVerts, GLenum mode=GL_QUADS );

template< typename T, typename U >
void draw( T* verts, size_t nVerts, int texture, U* coords, 
           GLenum mode=GL_QUADS );

// DEFINITIONS
template< typename T >
void draw( T* verts, size_t nVerts, GLenum mode )
{
    glEnableClientState( GL_VERTEX_ARRAY );

    glVertexPointer( 2, opengl_traits<T>::GL_TYPE, 0, verts );
    glDrawArrays( mode, 0, nVerts );

    glDisableClientState( GL_VERTEX_ARRAY );
}

template< typename T, typename U >
void draw( T* verts, size_t nVerts, int texture, U* coords, GLenum mode )
{
    if( texture ) {
        glEnable( GL_TEXTURE_2D );

        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        glBindTexture( GL_TEXTURE_2D, texture );
        glTexCoordPointer( 2, opengl_traits<U>::GL_TYPE, 0, coords );
    }

    draw( verts, nVerts, mode );

    if( texture ) {
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisable( GL_TEXTURE_2D );
    }
}

} // namespace draw
