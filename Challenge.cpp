
#include "Challenge.h"

#include "draw_shape.h"

Goal* Package::goal = 0;

const float Obsticle::SIZE = 15;
const float Goal::SIZE = 23;

Package::Package( const vector_type& pos, const vector_type& v )
    : Orbital( pos, vector_type(0,0) )
{
    activationDelay = 1;
    started = false;
    isMovable = false;
    reachedGoal = false;
}

void Package::move( int dt )
{
    if( !started && target && 
        magnitude(target->s-s) < RADIUS_TO_START+target->radius() ) 
    {
        started   = true;
        isMovable = true;
        isActive  = true;
    }

    Orbital::move( dt );
}

void Package::draw()
{
    Orbital::draw();

    if( !started ) {
        glDisable( GL_TEXTURE_2D );
        glTranslatef( s.x(), s.y(), 0 );
        glColor3f( 1, 1, 1 );
        draw_loop( RADIUS_TO_START, RADIUS_TO_START-5, 50 );
        glEnable( GL_TEXTURE_2D );
        glLoadIdentity();
    }
}

Color Package::color()
{
    return Color( 0.7, 0.5, 0.2, 1 );
}

void Package::collide_with( CircleActor& collider )
{
    if( &collider == goal ) {
        isMovable = false;
        reachedGoal = true;
    } else {
        deleteMe = true;
    }
}

Obsticle::Obsticle( const vector_type& pos, const vector_type& v )
    : Orbital( pos, vector_type(0,0) )
{
    isMovable = false;
    activationDelay = 0;
}


Color Obsticle::color()
{
    return Color( 1, 1, 1 );
}

Obsticle::value_type Obsticle::radius() const
{
    return SIZE;
}

void Obsticle::collide_with( CircleActor& collider )
{
    // This is not a stub. Do nothing on collision.
}

Goal::Goal( const vector_type& pos, const vector_type& v )
    : Orbital( pos, vector_type(0,0) )
{
    isMovable = false;
    activationDelay = 0;
}

Color Goal::color()
{
    return Color( 0.3, 1.8, 0.8 );
}

Goal::value_type Goal::radius() const
{
    return SIZE;
}

void Goal::collide_with( CircleActor& collider )
{
    // This is not a stub. Do nothing on collision.
}
