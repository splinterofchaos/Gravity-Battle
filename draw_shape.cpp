
#include "draw_shape.h"

#include "glpp.h"
#include "Vector.h"

#include <cmath>
#include <vector> // Using raw arrays may be smarter.

// An idea i had. Make all the draw functions objects. The arguments become
// stored variables. The interface this allows is the fallowing:
//     draw( circle(rad,verts) );
// That abstracts calculating the vertices from drawing them. It also abstracts
// /how/ they're drawn. Consider that each calc function can contain a GLenum,
// mode, specifying what draw mode the function draw should use. 
//
//  void draw( F f ) {
//      GLenum mode = F::GL_MODE;
//      buffer = f();
//      glEnableClientState( ... );
//      glVertexArrays( ... );
//      glDrawArrays( mode, 0, f.count() );
//      glDisableClientState( ... );
//  }
//
// More advanced versions should be able to use indices produced by f as well.

void draw_square( float size_over_2 )
{
    using namespace glpp;
    vertex( -size_over_2, -size_over_2 );
    vertex(  size_over_2, -size_over_2 );
    vertex(  size_over_2,  size_over_2 );
    vertex( -size_over_2,  size_over_2 );
}

void draw_rectangle( float w_over_2, float l_over_2 )
{
    using namespace glpp;
    vertex( -w_over_2, -l_over_2 );
    vertex(  w_over_2, -l_over_2 );
    vertex(  w_over_2,  l_over_2 );
    vertex( -w_over_2,  l_over_2 );
}

// Technique borrowed from http://slabode.exofire.net/circle_draw.shtml
void draw_circle( float radius, unsigned int nVerteces )
{
    const float ROTATION = (2 * 3.145) / nVerteces; // Change in theta.
    const float TANGENTAL_FACTOR = std::tan( ROTATION );
    const float RADIAL_FACTOR    = std::cos( ROTATION );

    Vector<float,2> rv; // Radial vector.
    rv.x( radius ); rv.y( 0.0f );

    std::vector< GLfloat > verteces;
    verteces.reserve( nVerteces );

    verteces.insert( verteces.end(), rv.begin(), rv.end() );

    // Since a circle can be drawn using GL_POLYGON, there is no need to draw
    // the last vertex which is the same as the first. Therefore, nVerteces-1
    // iterations is fine.
    for( unsigned int i=0; i < nVerteces-1; i++ )
    {
        Vector<float,2> tv; // Tangential vector.
        tv.x( -rv.y() ); tv.y( rv.x() );

        rv += tv * TANGENTAL_FACTOR;
        rv *= RADIAL_FACTOR;

        verteces.insert( verteces.end(), rv.begin(), rv.end() );
    }

    using namespace glpp;
    enableClientState( GL_VERTEX_ARRAY );
    vertexPointer( 2, 0, verteces.data() );
    drawArrays( GL_POLYGON, 0, nVerteces );
    disableClientState( GL_VERTEX_ARRAY );
}

// Mainly, same approach as above, but with two circles.
void draw_loop( float rad1, float rad2, unsigned int nVertecies )
{
    const float ROTATION = 2 * 3.145 / nVertecies;
    const float TANGENTAL_FACTOR = std::tan( ROTATION );
    const float RADIAL_FACTOR    = std::cos( ROTATION );

    Vector<float,2> rv1; // 1 radial vector.
    Vector<float,2> rv2; // 2 radial vector.

    rv1.x( rad1 ); rv1.y( 0.0f );
    rv2.x( rad2 ); rv1.y( 0.0f );

    std::vector< GLfloat > verteces;
    // Two extra vertices are needed since the start vertices are repeated at
    // the end.
    verteces.reserve( nVertecies * 2 + 2 );

    verteces.insert( verteces.end(), rv1.begin(), rv1.end() );
    verteces.insert( verteces.end(), rv2.begin(), rv2.end() );
    for( unsigned int i=0; i < nVertecies; i++ )
    {
        Vector<float,2> tv1; 
        tv1.x( -rv1.y() ); tv1.y( rv1.x() );

        Vector<float,2> tv2; 
        tv2.x( -rv2.y() ); tv2.y( rv2.x() );

        rv1 += tv1 * TANGENTAL_FACTOR;
        rv1 *= RADIAL_FACTOR;

        rv2 += tv2 * TANGENTAL_FACTOR;
        rv2 *= RADIAL_FACTOR;

        verteces.insert( verteces.end(), rv1.begin(), rv1.end() );
        verteces.insert( verteces.end(), rv2.begin(), rv2.end() );
    }

    using namespace glpp;
    enableClientState( GL_VERTEX_ARRAY );
    vertexPointer( 2, 0, verteces.data() );
    drawArrays( GL_QUAD_STRIP, 0, nVertecies*2 + 2 );
    disableClientState( GL_VERTEX_ARRAY );
}
