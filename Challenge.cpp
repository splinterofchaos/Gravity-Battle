
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

Package::value_type Package::mass()
{
    return 20;
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
    isActive = true;
    activationDelay = 0;
}


void Obsticle::move( float dt )
{
    collisionChecked = false;
    s += v*dt;

    state( Orbital::on_off_screen( state() ) );
}

Color Obsticle::color()
{
    return Color( 1, 1, 1 );
}

Obsticle::value_type Obsticle::radius() const
{
    return SIZE;
}

void Obsticle::collide_with( CircleActor& other )
{
    if( collisionChecked )
        return;

    Obsticle* otherPtr = dynamic_cast<Obsticle*>( &other );
    if( otherPtr )
        otherPtr->collisionChecked = true;

    if( &other == Package::goal.lock().get() )
        return;

    isMovable = true;
    other,isMovable = true;

    Vector<float,2> diff = other.s - s;
    float d       = magnitude( diff );
    float combRad = other.radius() + radius();

    // Move them off each other.
    s -= magnitude( diff, magnitude(diff) - combRad + 0.01 );

    Vector<float,2> mtd = diff * (((radius() + other.radius())-d)/d); 

    Vector<float,2> vel = v - other.v;
    float vn = vel * unit(mtd);

    if( vn < 0 )
        // They are moving away from each other. No extra work needed.
        return;

    float i = -0.9 * vn * ( mass() + other.mass() );
    Vector<float,2> impulse = mtd * i;

    v       += impulse / mass();
    other.v -= impulse / other.mass();
    return;

    if( ! isMovable )
    {
        isMovable = true;
        v = other.v;
    }
    else
    {
        if( ! other.isMovable )
        {
            other.v =  v / 2;
            v       = -v / 2;
        }
        else
        {
            v = -v;
            other.v = -v;
        }
    }
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
