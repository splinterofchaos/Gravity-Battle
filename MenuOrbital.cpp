
#include "MenuOrbital.h"
#include "Random.h"

#include <cmath>

MenuOrbital::MenuOrbital( const vector_type& pos, const vector_type& )
    : Orbital( pos, vector_type(0,0) )
{
    time = random( 0.0f, 3.0f );;
    angle = random_angle();

    isActive = false;
    activationDelay = ACTIVATION_DELAY;

    isDeadly = false;
}

CircleActor::State MenuOrbital::integrate( State state, int dt, value_type )
{
    stateTime  += float(dt) / 800;
    stateAngle += float(dt) / 1000;

    Orbital::SharedPlayerPtr target = Orbital::target.lock();

    value_type magnitude = cos( stateTime * 3 );
    value_type amplitude = 4000 / target->mass();

    vector_type oldS = state.s;
	vector_type oldV = state.v;

    state.s.x( std::cos(stateAngle) );
    state.s.y( std::sin(stateAngle) );
    state.s = state.s*amplitude*magnitude + target->s;

    // v is not needed, but setting it properly enables the velocity
    // arrow.
    state.v = (state.s - oldS) / 10;
	state.a = (state.v - oldV) / 10;

    return state;
}

void MenuOrbital::move( int dt )
{
    if( !isActive ) {
        activationDelay -= dt;
        if( activationDelay <= 0 )
            isActive = true;
    }

    stateTime  = time;
    stateAngle = angle;

    state( integrate(state(), dt, maxSpeed) );

    time  = stateTime;
    angle = stateAngle;
}

void MenuOrbital::draw()
{
    Orbital::draw();
}

void MenuOrbital::collide_with( CircleActor& )
{
    // Do nothing.
}
