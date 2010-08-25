
#include "glpp.h"

namespace glpp
{

#define COLOR_FUNC4( type, t ) \
    void color( GL##type r, GL##type g, GL##type, GL##type b, GL##type a ) \
    { \
        glColor4##t( r, g, b, a ); \
    }
COLOR_FUNC4( short,  s );
COLOR_FUNC4( int,    i );
COLOR_FUNC4( float,  f );
COLOR_FUNC4( double, d );
#undef COLOR_FUNC4

void enableClientState( GLenum cap )
{
    glEnableClientState( cap );
}

void drawArrays( GLenum mode, GLint first, GLsizei count )
{
    glDrawArrays( mode, first, count );
}

void disableClientState( GLenum cap )
{
    glDisableClientState( cap );
}

void loadIdentity()
{
    glLoadIdentity();
}

void translate( GLfloat  x, GLfloat  y, GLfloat  z ) { glTranslatef(x,y,z); }
void translate( GLdouble x, GLdouble y, GLdouble z ) { glTranslated(x,y,z); }

#define VERTEX_FUNC2( type, t ) \
    void vertex( GL##type x, GL##type y ) { glVertex2##t( x, y ); }
VERTEX_FUNC2( float, f );
#undef VERTEX_FUNC2

#define VERTEX_FUNC3( type, t ) \
    void vertex( GL##type x, GL##type y, GL##type z ) { glVertex3##t( x, y, z ); }
VERTEX_FUNC3( float, f );
#undef VERTEX_FUNC3

void vertex( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
    glVertex4f( x, y, z, w );
}

#define VERTEX_FUNCV( type, t, N ) \
    void vertex( GL##type (&v)[N] ) { glVertex##N##t##v( v ); }
VERTEX_FUNCV( float, f, 2 );
VERTEX_FUNCV( float, f, 3 );
VERTEX_FUNCV( float, f, 4 );
#undef VERTEX_FUNCV

} // namespace glpp
