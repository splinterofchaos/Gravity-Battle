
#include "Particle.h"
#include "Random.h"

#include <cmath>

Particle::Particle( const vector_type& pos, const vector_type& v, 
                    value_type minSpeed, value_type maxSpeed, float scale,
                    const Color& c )   
    : Actor( pos ), c( c ), scale( scale )
{
    value_type speed = random( minSpeed, maxSpeed );
    float angle = random_angle();

    this->v.x( std::cos(angle) * speed );
    this->v.y( std::sin(angle) * speed );

    this->v += v;
}   

void Particle::draw()
{
    glTranslatef( s.x(), s.y(), 1 );

    float verts[] = { 
        -scale, -scale,
         scale, -scale,
         scale,  scale,        
        -scale,  scale,
    };

    int texCoords[] = {
        0, 0,
        1, 0,
        1, 1, 
        0, 1
    };

    glEnable( GL_TEXTURE_2D );
    glColor3f( c.r(), c.g(), c.b() );

    glBindTexture( GL_TEXTURE_2D, 1 );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    {
        glTexCoordPointer( 2, GL_INT, 0, texCoords );
        glVertexPointer( 2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_QUADS, 0, 4 );

    }
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );

    glLoadIdentity();
}
