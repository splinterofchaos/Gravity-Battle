
#include "Challenge.h"

#include "draw_shape.h"
#include "Draw.h"

Package::WeakGoalPtr Package::goal;

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
        glDisable( GL_TEXTURE_2D );
        glTranslatef( s.x(), s.y(), 0 );
        glColor3f( 1, 1, 1 );

        const int N_VERTS = 4;

        std::vector<float> circle1( N_VERTS*2 + 2 );

        // TODO: The fallowing just plain doesn't work. A circle is supposed to
        // be drawn, but instead... just run the code as-is to see. Either the
        // bug is in Draw.h's draw::circle or draw::draw. To remove the bug,
        // call draw_shape.h's draw_circle instead. However, i am migrating to
        // Draw.h, so the bug stays until Draw.h is bug free.

        // Static initialization of loop.
        auto end1 = draw::circle ( 
            circle1.begin(), circle1.size(), 
            RADIUS_TO_START
        );

        draw::draw( circle1.data(), N_VERTS, GL_LINE_STRIP );

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
    if( &collider == goal.lock().get() ) {
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
