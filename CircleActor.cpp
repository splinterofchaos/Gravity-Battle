
#include "CircleActor.h"

void CircleActor::state( const State& state )
{
    s = state.s;
    v = state.v;
    a = state.a;
}

CircleActor::State CircleActor::state()
{
    State st;
    st.s = s;
    st.v = v;
    st.a = a;
    return st;
}

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

CircleActor::State CircleActor::on_off_screen( State state )
{
    if( state.s.x() - radius() < Arena::minX )
        state.s.x( Arena::minX + radius() );
    else if( state.s.x() + radius() > Arena::maxX )
        state.s.x( Arena::maxX - radius() );

    if( state.s.y() - radius() < Arena::minY )
        state.s.y( Arena::minY + radius() );
    else if( state.s.y() + radius() > Arena::maxY )
        state.s.y( Arena::maxY - radius() );

    return state;
}

CircleActor::State CircleActor::integrate( State state, int dt, value_type maxSpeed )
{
    if( !isMovable )
        return state;

    state = on_off_screen( state );

    Actor::simple_integration( state.s, state.v, state.a, dt, maxSpeed );

    return state;
}

void CircleActor::move( int dt, value_type maxSpeed )
{
    previousS = s;

    state( integrate(state(), dt, maxSpeed) );
}

bool collision( const CircleActor& c1, const CircleActor& c2 )
{
    return c1.isActive && c2.isActive && 
        magnitude(c1.s - c2.s) < c1.radius() + c2.radius();
}
