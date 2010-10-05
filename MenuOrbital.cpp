
#include "MenuOrbital.h"
#include "Random.h"

#include <cmath>

MenuOrbital::MenuOrbital( const vector_type& pos, const vector_type& v )
    : Orbital( pos, vector_type(0,0) )
{
    time = 0;
    angle = random_angle();

    isActive = false;
    activationDelay = ACTIVATION_DELAY;

    isDeadly = false;
}

void MenuOrbital::move( int dt )
{
    if( !isActive ) {
        activationDelay -= dt;
        if( activationDelay <= 0 )
            isActive = true;
    }

    time  += float(dt) / 800;
    angle += float(dt) / 1000;

    value_type magnitude = cos( time * 3 );
    value_type amplitude = 4000 / target->mass();

    vector_type oldS = s;
	vector_type oldV = v;

    s.x( std::cos(angle) );
    s.y( std::sin(angle) );
    s = s*amplitude*magnitude + target->s;

    // v is not needed, but setting it properly enables the velocity arrow.
    v = (s - oldS) / 10;
	a = (v - oldV) / 10;
}

void MenuOrbital::draw()
{
    Orbital::draw();
}

void MenuOrbital::collide_with( CircleActor& collider )
{
    // Do nothing.
}
