
#include "CircleActor.h"

CircleActor::CircleActor()
{
    isActive = true;
    isMovable = true;
}

CircleActor::CircleActor( const CircleActor::vector_type& position )
    : Actor( position )
{
    isActive = true;
    isMovable = true;
}

void CircleActor::on_off_screen()
{
    if( s.x() - radius() < 0 )
        s.x( s.x() + (radius()-s.x()) );
    else if( s.x() + radius() > Arena::maxX )
        s.x( s.x() - ((s.x()+radius()) - Arena::maxX) );

    if( s.y() - radius() < 0 )
        s.y( s.y() + (radius()-s.y()) );
    else if( s.y() + radius() > Arena::maxY )
        s.y( s.y() - ((s.y()+radius()) - Arena::maxY) );
}

void CircleActor::move( int dt, value_type maxSpeed )
{
    on_off_screen();

    Actor::simple_integration( s, v, a, dt, maxSpeed );
}

bool collision( const CircleActor& c1, const CircleActor& c2 )
{
    return c1.isActive && c2.isActive && 
        magnitude(c1.s - c2.s) < c1.radius() + c2.radius();
}
