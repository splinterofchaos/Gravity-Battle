
#include "MenuOrbital.h"

#include <cmath>

MenuOrbital::MenuOrbital( const vector_type& pos, const vector_type& v )
    : Orbital( pos, vector_type(0,0) )
{
    time = 0;
    angle = 0;

    activationDelay = 1;
}

void MenuOrbital::move( int dt )
{
    time  += float(dt) / 800;
    angle += float(dt) / 1000;

    value_type magnitude = cos( time * 3 );
    value_type amplitude = 250;

    vector_type oldS = s;
    s.x( std::cos(angle) );
    s.y( std::sin(angle) );
    s = s*amplitude*magnitude + target->s;

    // v is not needed, but setting it properly enables the velocity arrow.
    v = s - oldS;
}

void MenuOrbital::draw()
{
    Orbital::draw();
}
