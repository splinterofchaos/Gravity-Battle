 
#include "Orbitals.h"
#include "Random.h"
#include "Draw.h"

#include <algorithm>
#include <cmath>

Texture Orbital::image;
Orbital::Attractors Orbital::attractors;
unsigned int Orbital::predictionLength    = 0;
unsigned int Orbital::predictionPrecision = 17;
unsigned int Orbital::gravityLine         = 0;
bool         Orbital::velocityArrow       = false;
bool         Orbital::accelerationArrow   = false;

Orbital::WeakPlayerPtr Orbital::target;
Orbital::WeakPlayerPtr Orbital::target2;

Sound Orbital::wallSfx[ N_WALL_SFX ];
Sound Orbital::birthSfx[ N_BIRTH_SFX ];
Sound Stopper::switchSfx[ N_SWITCHS ];

Orbital::Orbital( const Orbital::vector_type& pos,
                  const Orbital::vector_type& vel, bool playSound )
    : CircleActor( pos )
{
    v = vel;
    activationDelay = ACTIVATION_DELAY;
    isActive = false;

    colorIntensity = random( 6, 10 ) / 10.0f;

    g = vector_type( 0, 0 );

    if( playSound )
        birthSfx[ random(0, N_BIRTH_SFX) ].play();

    hitWall = false;
}

CircleActor::State Orbital::on_off_screen( State state )
{
    hitWall = false;

    if( state.s.x() - radius() < Arena::minX && state.v.x() < 0 ) {
        state.v.x( -state.v.x() * BOUNCINESS );
        hitWall = true;
    } else if( state.s.x() + radius() > Arena::maxX && state.v.x() > 0 ) {
        state.v.x( -state.v.x() * BOUNCINESS );
        hitWall = true;
    }

    if( state.s.y() - radius() < Arena::minY && state.v.y() < 0 ) {
        state.v.y( -state.v.y() * BOUNCINESS );
        hitWall = true;
    } else if( state.s.y() + radius() > Arena::maxY && state.v.y() > 0 ) {
        state.v.y( -state.v.y() * BOUNCINESS );
        hitWall = true;
    }

    return state;
}

Orbital::vector_type Orbital::acceleration( const vector_type& r )
{

    return magnitude ( 
        r, target.lock()->mass() * (1.0f/90.0f) / magnitude(r) * Arena::scale 
    );
}

CircleActor::State Orbital::integrate( State state, int dt, value_type maxSpeed )
{
    if( !isMovable )
        return state;

    if( isActive )
    {
        g *= 0;

        for( size_t i=0; i < attractors.size(); i++ )
        {
            std::tr1::shared_ptr<CircleActor> attr = attractors[i].lock();
            if( attr && attr->isActive && attr.get() != this ) 
            {
                vector_type r = attr->s - state.s;
                g += magnitude ( 
                    r, attr->mass() * g_multiplier() / g_dist(r) * Arena::scale
                );
            }
        }

        state.a = g;
    }
    else
    {
        state.a *= 0;
    }

    return CircleActor::integrate( state, dt, maxSpeed );
}

void Orbital::move( int dt )
{
    if( !isActive )
    {
        activationDelay -= dt;
        if( activationDelay <= 0 )
            isActive = true;
    }

    vector_type vOrig = v;

    // Will call Orbital::integrate.
    CircleActor::move( dt );

    if( hitWall )
        wallSfx[ random(0, N_WALL_SFX) ].play();
}

void Orbital::draw_impl( float* verts, float zRotation, bool extra )
{
    glTranslatef( s.x(), s.y(), 0 );
    glRotatef( zRotation, 0, 0, 1 );

    GLfloat activationProgress;
    if( isActive )
        activationProgress = 1;
    else
        activationProgress =  0.5 + 0.5 * std::cos ( 
            (float)activationDelay * (3.14/ACTIVATION_DELAY) 
        );

    for( int i=0; i < 8; i++ )
        verts[i] *= activationProgress;

    float texCoords[] = {
        0, 0,
        1, 0,
        1, 1, 
        0, 1
    };

    Color c = color();
    if( ! isActive )
        c = { 1, 1, 1, 1 };

    glColor4f( c.r(), c.g(), c.b(), c.a() );

    glBindTexture( GL_TEXTURE_2D, image.handle() );

    draw::draw( verts, 4, image.handle(), texCoords );

    glLoadIdentity();

    SharedPlayerPtr target  = Orbital::target.lock();
    SharedPlayerPtr target2 = Orbital::target2.lock();
    if( extra && isMovable )
    {
        if( gravityLine ) 
        {
            for( size_t i=0; i < attractors.size(); i++ ) 
                if( auto attr = attractors[i].lock() )
                {
                    vector_type accelerationLine[] = {
                        attr->s,
                        s,
                    };

                    draw::draw( &accelerationLine[0][0], 2, image.handle(), 
                                texCoords, GL_LINE_STRIP );
                }
        }

        const unsigned int NUM_PREDICTIONS = predictionLength;
        unsigned int actualPredictions = 0;
        Vector<float,2> pathOfOrbit[NUM_PREDICTIONS];
        Color           pathColors[NUM_PREDICTIONS];

        State p; // prediction

        p.s = s; p.v = v;
        pathOfOrbit[0] = p.s;

        for( size_t i=1; i < NUM_PREDICTIONS; i++, actualPredictions++ ) 
        {
            for( size_t j=0; j < predictionPrecision; j++ )
            {
                p = integrate( p, 4 );

                // If this state will collide with an attractor, break.
                for( size_t k=0; k < attractors.size(); k++ ) 
                {
                    auto attr = attractors[k].lock();
                    if( ! attr || attr.get() == this )
                        continue;

                    float combRad = attr->radius() + radius();
                    vector_type r = attr->s - p.s;

                    if( magnitude(r) < combRad )
                    {
                        // Exit both inner and outer loops.
                        i = NUM_PREDICTIONS;
                        j = predictionPrecision;

                        // No new prediction. Undo increment.
                        actualPredictions--;

                        break;
                    } 
                }

                if( true ) {
                    p = on_off_screen( p );
                }
            }

            pathOfOrbit[i] = p.s;
            if( false ) {
                pathColors[i] = c * magnitude(p.v) * 4;
            } else {
                pathColors[i] = 
                    c * (float(NUM_PREDICTIONS-i)/NUM_PREDICTIONS) * 4;
                pathColors[i].a( 0.7 );
            }
        } // For( i = (0,NUM_PREDICTIONS] )

        glDisable( GL_TEXTURE_2D );
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, 0, pathColors );
        draw::draw( &pathOfOrbit[0][0], actualPredictions, GL_LINES );
        glDisableClientState( GL_COLOR_ARRAY );


        if( velocityArrow ) {
            const float LENGTH_MULT = 300;
            const float WIDTH_MULT  = 20;
            
            // Vector with magnitude/direction of the head of the arrow.
            const vector_type HEAD = magnitude( v, magnitude(v*LENGTH_MULT) );

            vector_type velocityLine[] = {
                s,
                s + HEAD*(4.0/5) + clockwise_tangent(v)*WIDTH_MULT,
                s + HEAD,
                velocityLine[1] - clockwise_tangent(v)*WIDTH_MULT*2
            };

            // Draw arrow over this.
            glTranslatef( 0, 0, 1 );

            Color c = color() / 1.5;
            glColor4f( c.r(), c.g(), c.b(), 0.5 );

            draw::draw( &velocityLine[0][0], 4, GL_POLYGON );

            glLoadIdentity();
        }

        if( accelerationArrow ) {
            const float LENGTH_MULT = 220000;
            const float WIDTH_MULT  = 20000;
            const vector_type HEAD = magnitude( a, magnitude(a*LENGTH_MULT) );
            vector_type accelerationLine[] = {
                s,
                s + HEAD*(4.0/5) + clockwise_tangent(a)*WIDTH_MULT,
                s + HEAD,
                accelerationLine[1] - clockwise_tangent(a)*WIDTH_MULT*2
            };
            // Draw arrow over this.
            glTranslatef( 0, 0, 1 );

            Color c = color() * 4;
            glColor4f( c.r(), c.g(), c.b(), 0.5 );

            draw::draw( &accelerationLine[0][0], 4, GL_LINE_LOOP );

            glLoadIdentity();
        }
    } // if extra

}

void Orbital::draw()
{
    float verts[] = { 
        -radius(), -radius(),
        radius(), -radius(),
        radius(),  radius(),        
        -radius(),  radius(),
    };

    draw_impl( verts, 0 );
}

int Orbital::score_value()
{
    return 2;
}

Orbital::value_type Orbital::radius() const
{
    return RADIUS * Arena::scale;
}

Orbital::value_type Orbital::mass() const
{
    return 10;
}

Color Orbital::color()
{
    Color c = Color( .4f, .4f, 1.0, 1.0f ) * colorIntensity;
    c.a( 1 );
    return c;
}

Orbital::value_type Orbital::g_multiplier()
{
    return 1.0f / 90.0f;
}

Orbital::value_type Orbital::g_dist( const vector_type& r )
{
    return magnitude( r );
}

void Orbital::collide_with( CircleActor& collider )
{
    deleteMe = true;
}

Twister::Twister( const Orbital::vector_type& pos, 
                  const Orbital::vector_type& v )
    : Orbital( pos, v )
{
    angleAcc = 0;
    angleVel = random( 0.1f, 0.9f );
    angle = random_angle() * (180/3.145);
}

CircleActor::State Twister::on_off_screen( State state )
{
    State st = Orbital::on_off_screen( state );

    if( v != st.v )
        angleVel = -angleVel/3;

    return st;
}

Twister::vector_type Twister::acceleration( const vector_type& r )
{
    return magnitude( r, target.lock()->mass() / (r*r) ) * Arena::scale;
}

void Twister::move( int dt )
{
    Orbital::move( dt );

    const float ROTATION_MULTIPLIER =  1.0f / 1.0f;
    angleAcc = cross(a,v) * ROTATION_MULTIPLIER / magnitude(v); 

    // Integrate.
    // Actor::simple_integration does the same thing, but for
    // vectors. TODO: Make Actor::simple_integration more generic.
    angleVel += angleAcc*dt;
    angle    += angleVel*dt + angleAcc*dt*dt;
}

void Twister::draw()
{
    float verts[] = { 
        -radius(), -radius()/2,
        radius(), -radius()/2,
        radius(),  radius()/2,        
        -radius(),  radius()/2,
    };

    draw_impl( verts, angle );
}

Color Twister::color()
{
    Color c = Color( 1.0f, 0.1f, 0.1f, 1.0f ) * colorIntensity;
    c.a( 1 );
    return c;
}

Twister::value_type Twister::g_dist( const vector_type& r )
{
    return r*r;
}

Twister::value_type Twister::g_multiplier()
{
    return 1;
}

int Twister::score_value()
{
    return 6;
}

Stopper::Stopper( const vector_type& pos, const vector_type& v )
    : Orbital( pos, v )
{
    std::fill_n( timesOfCollisions, N_COLLISIONS_PER_SEC, 101 );
}

Stopper::vector_type Stopper::acceleration( const vector_type& r )
{
    return magnitude (
        r, target.lock()->mass() * (1.0f/220.0f) / magnitude(r) 
    ) * Arena::scale;
}

int Stopper::score_value()
{
    return 4;
}

void Stopper::move( int dt )
{
    // Update the collision times.
    for( size_t i=0; i < N_COLLISIONS_PER_SEC; i++ )
        timesOfCollisions[i] += dt;

    if( isMovable )
        Orbital::move( dt );

    isDeadly = isMovable;
}

void Stopper::draw()
{
    if( isMovable ) {
        isMovable = false;
        Orbital::draw();
        isMovable = true;
    }

    glTranslatef( 0, 0, 1 );
    Orbital::draw();
}

Stopper::value_type Stopper::radius() const
{
    if( isMovable )
        return RADIUS * Arena::scale;
    else
        return STOPPED_RADIUS * Arena::scale;
}

Stopper::value_type Stopper::mass() const
{
    return 15 * Arena::scale;
}

void Stopper::collide_with( CircleActor& collider )
{
	const int COLLISION_DELAY = 160;

    // Rotate the times.
    std::copy ( 
        timesOfCollisions, timesOfCollisions+N_COLLISIONS_PER_SEC-1,
        timesOfCollisions+1 
    );
    timesOfCollisions[0] = 0;

    // CircleActor** thisCollider = 
    //     std::find( lastColiders, lastColiders+N_LAST_COLLIDERS, &collider );

    std::copy ( 
        lastColiders, lastColiders+N_LAST_COLLIDERS-1,
        lastColiders+1 
    );
    lastColiders[0] = &collider;

    bool presentState = isMovable;

    if( isMovable ) 
    {
        isMovable = false;
    }
    else
    {
        // If collider is player (only type with radius==25), die.
        if( &collider == Orbital::target.lock().get() || 
            &collider == Orbital::target2.lock().get() ) 
        {
            deleteMe = true;
        } 
        else if( (timesOfCollisions[4] <= COLLISION_DELAY*4) &&
                   (this > &collider) ) 
        {
            // Must be colliding over and over with another stopper.
            deleteMe = true;
        }
        else 
        {
            // Non-players will make it go again.
            isMovable = true;

            // Transfer momentum fro collider to this.
            v = collider.v * collider.mass() / mass();
        }
    }

    if( isMovable != presentState )
        switchSfx[ random(0, N_SWITCHS) ].play();
}

Color Stopper::color()
{
    Color c;

    if( !isMovable ) {
        c = Color( 0.6, 0.6, 0.6, 0.6 ) * colorIntensity;
        c.a( 1 );
    } else {
        c = Color( 1, 1, 1, 0.65 );
    }

    return c;
}

Stopper::value_type Stopper::g_multiplier()
{
    return 1.0f / 220.0f;
}

Sticker::Sticker( const vector_type& pos, const vector_type& v )
	: Orbital( pos, v )
{
}

Sticker::vector_type Sticker::acceleration( const vector_type& r )
{
	return magnitude( r, r*r / 50000000 );
}

Sticker::value_type Sticker::radius() const
{
	return RADIUS;
}

Color Sticker::color()
{
    Color c = Color( 0.3f, 1.0f, 1.0f, 1.0f ) * colorIntensity;
    c.a( 1 );
    return c;
}
