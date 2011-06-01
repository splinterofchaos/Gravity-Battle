
#include "Challenge.h"

#include "draw_shape.h"
#include "Draw.h"

Package::WeakGoalPtr Package::goal;

const float Package::RADIUS_TO_START = 200;
const float Obsticle::SIZE = 15;
const float Goal::SIZE = 23;

Package::Package( const vector_type& pos, const vector_type& )
    : Orbital( pos, vector_type(0,0), false )
{
    activationDelay = 1;
    started = false;
    isMovable = false;
    reachedGoal = false;
}

Package::State Package::on_off_screen( State s )
{
    if( isActive )
        return Orbital::on_off_screen( s );
    return s;
}

void Package::move( float dt )
{
    std::tr1::shared_ptr<Player> target = Orbital::target.lock();
    if( !started && target && 
        magnitude(target->s-s) < RADIUS_TO_START+target->radius() ) 
    {
        started   = true;
        isMovable = true;
        isActive  = true;
    }

    Orbital::move( dt );
}

#include <fstream>
void Package::draw()
{
    Orbital::draw();

    if( !started ) {
        glTranslatef( s.x(), s.y(), 0 );
        glColor3f( 1, 1, 1 );

        const int N_VERTS = 40;

        draw_loop( RADIUS_TO_START, RADIUS_TO_START + 10, N_VERTS );

        glLoadIdentity();
    }
}

Color Package::color()
{
    return Color( 0.7, 0.5, 0.2, 1 );
}

void Package::collide_with( CircleActor& collider )
{
    if( &collider == goal.lock().get() ) {
        isMovable = false;
        reachedGoal = true;
    } else {
        deleteMe = true;
    }
}

Obsticle::Obsticle( const vector_type& pos, const vector_type& )
    : Orbital( pos, vector_type(0,0), false )
{
    isMovable = false;
    activationDelay = 0;
}

Obsticle::State Obsticle::on_off_screen( State s )
{
    return s;
}

Color Obsticle::color()
{
    return Color( 1, 1, 1 );
}

Obsticle::value_type Obsticle::radius() const
{
    return SIZE;
}

void Obsticle::collide_with( CircleActor& )
{
    // This is not a stub. Do nothing on collision.
}

Goal::Goal( const vector_type& pos, const vector_type& )
    : Orbital( pos, vector_type(0,0), false )
{
    isMovable = false;
    activationDelay = 0;
}

Goal::State Goal::on_off_screen( State s )
{
    return s;
}

Color Goal::color()
{
    return Color( 0.3, 1.8, 0.8 );
}

Goal::value_type Goal::radius() const
{
    return SIZE;
}

void Goal::collide_with( CircleActor& )
{
    // This is not a stub. Do nothing on collision.
}
