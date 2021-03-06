
#include "Particle.h"
#include "Random.h"
#include "Draw.h"

#include <cmath>

bool  Particle::gravityField = false;
float Particle::opacity = 30;
float Particle::sizeMult = 1;

Particle::Particle( const vector_type& pos, const vector_type& v, 
                    value_type minSpeed, value_type maxSpeed, float scale,
                    const Color& c )   
    : Actor( pos ), c( c ), scale( scale*sizeMult ), maxSpeed( maxSpeed )
{
    value_type speed = random( minSpeed, maxSpeed );
    float angle = random_angle();

    this->v.x( std::cos(angle) * speed );
    this->v.y( std::sin(angle) * speed );

    this->v += v;

    this->c *= random( 0.1f, 1.5f );
	this->c.a( 0.7 );

    isVisible = true;
}   

void Particle::draw()
{
    // If =a, particles will represent the gravities affecting them.
    // If =v, they will be drawn according to their motion.
    vector_type& direction = gravityField? a : v;
    float max = gravityField? 0.001f : maxSpeed;

    float angle = std::atan2( direction.y(), direction.x() ) * (180/3.145f);
    float widthRatio = 1.0;

    if( gravityField )
        widthRatio += max / magnitude(direction) * 11;
    else
        widthRatio += magnitude(direction) / max * 11;

    if( widthRatio > 15 )
        widthRatio = 15;

    if( direction == a )
        angle += 90;

    float verts[] = { 
        -scale * widthRatio, -scale,
         scale * widthRatio, -scale,
         scale * widthRatio,  scale,        
        -scale * widthRatio,  scale,
    };

    glTranslatef( s.x(), s.y(), 1 );
    glColor4f( c.r(), c.g(), c.b(), opacity );
    glRotatef( angle, 0, 0, 1 );

    draw::draw( verts, 4 );

    glDisable( GL_DEPTH_TEST );
    glLoadIdentity();
}
