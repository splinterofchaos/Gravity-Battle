
#include "CircleActor.h"

void CircleActor::init()
{
    isActive = true;
    isMovable = true;
    isDeadly = true;
}

CircleActor::CircleActor()
{
    init();
}

CircleActor::CircleActor( const CircleActor::vector_type& position )
    : Actor( position )
{
    init();
}

void CircleActor::on_off_screen()
{
    if( s.x() - radius() < Arena::minX )
        s.x( Arena::minX + radius() );
    else if( s.x() + radius() > Arena::maxX )
        s.x( Arena::maxX - radius() );

    if( s.y() - radius() < Arena::minY )
        s.y( Arena::minY + radius() );
    else if( s.y() + radius() > Arena::maxY )
        s.y( Arena::maxY - radius() );
}

void CircleActor::move( int dt, value_type maxSpeed )
{
    on_off_screen();

    previousS = s;
    Actor::simple_integration( s, v, a, dt, maxSpeed );
}

bool collision( const CircleActor& c1, const CircleActor& c2 )
{
    return c1.isActive && c2.isActive && 
        magnitude(c1.s - c2.s) < c1.radius() + c2.radius();
}
