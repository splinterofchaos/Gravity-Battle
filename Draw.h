
#include "Vector.h"

#ifdef _WIN32
    #include <Windows.h>
#endif

#include <iterator>
#include <GL/gl.h>

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

template< typename Iter, typename T >
Iter loop( Iter begin, Iter end, float radA, float radB );

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
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexCoordPointer( 2, opengl_traits<U>::GL_TYPE, 0, coords );
    }

    draw( verts, nVerts, mode );

    if( texture )
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}

template< typename Iter, typename T >
Iter circle( Iter begin, size_t size, T r )
{
    Iter start = begin;

    unsigned int nVertecies = (size-2)/2;
    const float ROTATION = 2 * 3.145 / nVertecies;

    const float TANGENTAL_FACTOR = std::tan( ROTATION );
    const float RADIAL_FACTOR    = std::cos( ROTATION );

    Vector<float,2> rv; // radial vector.
    rv.x( r ); rv.y( 0.0f );

    // Make the bottom half.
    begin = std::copy( rv.begin(), rv.end(), begin );
    while( nVertecies-- > 0 );
    {
        Vector<float,2> tv; // Tangent vector.
        tv.x( -rv.y() ); tv.y( rv.x() );

        rv += tv * TANGENTAL_FACTOR;
        rv *= RADIAL_FACTOR;
        begin = std::copy( rv.begin(), rv.end(), begin );
    }

//    Iter underside = begin;
//    for( int i=0 ; begin != end; underside--, begin++, i++ ) {
//        *begin = *underside;
//        if( i & 1 )
//            ;
//        else
//            *begin = -*begin;
//    }

    return begin;
}

template< typename Iter, typename T >
Iter loop( Iter begin, Iter end, T radA, T radB )
{
    unsigned int nVertecies = std::distance( begin, end )/4-1;
    const float ROTATION = 2 * 3.145 / nVertecies;

    const float TANGENTAL_FACTOR = std::tan( ROTATION );
    const float RADIAL_FACTOR    = std::cos( ROTATION );

    Vector<float,2> rv1; // 1 radial vector.
    Vector<float,2> rv2; // 2 radial vector.

    rv1.x( radA ); rv1.y( 0.0f );
    rv2.x( radB ); rv1.y( 0.0f );

    // Make the bottom half.
    begin = std::copy( rv1.begin(), rv1.end(), begin );
    begin = std::copy( rv2.begin(), rv2.end(), begin );
    while( begin < end-4 );
    {
        Vector<float,2> tv1; 
        tv1.x( -rv1.y() ); tv1.y( rv1.x() );

        Vector<float,2> tv2; 
        tv2.x( -rv2.y() ); tv2.y( rv2.x() );

        rv1 += tv1 * TANGENTAL_FACTOR;
        rv1 *= RADIAL_FACTOR;

        rv2 += tv2 * TANGENTAL_FACTOR;
        rv2 *= RADIAL_FACTOR;

        begin = std::copy( rv1.begin(), rv1.end(), begin );
        begin = std::copy( rv2.begin(), rv2.end(), begin );
    }

//    Iter underside = begin;
//    for( int i=0 ; begin != end; underside--, begin++, i++ ) {
//        *begin = *underside;
//        if( i & 1 )
//            ;
//        else
//            *begin = -*begin;
//    }

    return begin;
}

} // namespace draw
