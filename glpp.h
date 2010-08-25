
#pragma once

#include <GL/gl.h>

namespace glpp
{

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

#define COLOR_FUNC4( type, t ) \
    void color( GL##type r, GL##type g, GL##type, GL##type b, \
                GL##type a = GL##type(1) ); 
COLOR_FUNC4( short,  s );
COLOR_FUNC4( int,    i );
COLOR_FUNC4( float,  f );
COLOR_FUNC4( double, d );
#undef COLOR_FUNC4

void enableClientState( GLenum );
void drawArrays( GLenum mode, GLint first, GLsizei count );
void disableClientState( GLenum );

void loadIdentity();

void translate( GLfloat  x, GLfloat  y, GLfloat  z );
void translate( GLdouble x, GLdouble y, GLdouble z );

template< class T >
void vertexPointer( GLint size, GLsizei stride, const T* pointer )
{
    glVertexPointer( size, opengl_traits<T>::GL_TYPE, stride, pointer );
};

#define VERTEX_FUNC2( type, t ) \
    void vertex( GL##type x, GL##type y );
VERTEX_FUNC2( float, f );
#undef VERTEX_FUNC2

#define VERTEX_FUNC3( type, t ) \
    void vertex( GL##type x, GL##type y, GL##type z );
VERTEX_FUNC3( float, f );
#undef VERTEX_FUNC3

void vertex( GLfloat x, GLfloat y, GLfloat z, GLfloat w );

#define VERTEX_FUNCV( type, t, N ) \
    void vertex( GL##type (&v)[N] );
VERTEX_FUNCV( float, f, 2 );
VERTEX_FUNCV( float, f, 3 );
VERTEX_FUNCV( float, f, 4 );
#undef VERTEX_FUNCV

} // namespace glpp
