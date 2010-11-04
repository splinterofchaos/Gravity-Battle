
#include "Particle.h"
#include "Random.h"
#include "Draw.h"

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

    this->c *= random( 0.1f, 1.5f );
	this->c.a( 0.7 );
}   

void Particle::draw()
{
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

    glTranslatef( s.x(), s.y(), 1 );

    glColor4f( c.r(), c.g(), c.b(), c.a() );

    draw::draw( verts, 4, 1, texCoords );

    glDisable( GL_DEPTH_TEST );
    glLoadIdentity();
}
