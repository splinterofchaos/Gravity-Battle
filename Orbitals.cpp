 
#include "Orbitals.h"
#include "Random.h"

Texture Orbital::image;

const Orbital::value_type Orbital::RADIUS = 18;

Player* Orbital::target = 0;

Orbital::Orbital( const Orbital::vector_type& pos, const Orbital::vector_type& vel )
    : CircleActor( pos )
{
    v = vel;
    activationDelay = ACTIVATION_DELAY;
    isActive = false;

    colorIntensity = random( 0.6f, 1.0f );
}

void Orbital::on_off_screen()
{
    const float BOUNCINESS = 0.8;

    if( s.x() - radius() < 0 && v.x() < 0 )
        v.x( -v.x() * BOUNCINESS );
    else if( s.x() + radius() > Arena::maxX && v.x() > 0 )
        v.x( -v.x() * BOUNCINESS );

    if( s.y() - radius() < 0 && v.y() < 0 )
        v.y( -v.y() * BOUNCINESS );
    else if( s.y() + radius() > Arena::maxY && v.y() > 0 )
        v.y( -v.y() * BOUNCINESS );
}

Orbital::vector_type Orbital::acceleration( const vector_type& r )
{
    return magnitude( r, target->mass() * (1.0f/100.0f) / magnitude(r) );
}

void Orbital::move( int dt )
{
    if( !isActive )
    {
        activationDelay -= dt;
        if( activationDelay <= 0 )
            isActive = true;
    }

    if( isActive && target )
    {
        // Orbit the target.
        const vector_type r = target->s - s;
        a = acceleration( r );
    }
    else
    {
        a = vector_type( 0, 0 );
    }

    CircleActor::move( dt );
}

void Orbital::draw_impl( float* verts, float zRotation )
{
    glTranslatef( s.x(), s.y(), 0 );
    glRotatef( zRotation, 0, 0, 1 );

    GLfloat activationProgress;
    if( isActive )
        activationProgress = 1;
    else
        activationProgress = 1.0f - (float)activationDelay / ACTIVATION_DELAY;

    for( int i=0; i < 8; i++ )
        verts[i] *= activationProgress;

    float texCoords[] = {
        0, 0,
        1, 0,
        1, 1, 
        0, 1
    };

    Color c = color() * activationProgress;

    glEnable( GL_TEXTURE_2D );
    glColor4f( c.r(), c.g(), c.b(), c.a() );

    glBindTexture( GL_TEXTURE_2D, image.handle() );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    {
        glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );
        glVertexPointer(   2, GL_FLOAT, 0, verts     );
        glDrawArrays( GL_QUADS, 0, 4 );

        glLoadIdentity();

        if( target ) {
            float accelerationLine[] = {
                s.x(), s.y(),
                target->s.x(), target->s.y()
            };

            glVertexPointer( 2, GL_FLOAT, 0, accelerationLine );
            glDrawArrays( GL_LINES, 0, 2 );

            const unsigned int NUM_PREDICTIONS = 900;
            Vector<float,2> pathOfOrbit[NUM_PREDICTIONS];

            struct Prediction {
                vector_type s, v, a;
            } p;

            p.s = s; p.v = v;
            pathOfOrbit[0] = p.s;

            for( size_t i=1; i < NUM_PREDICTIONS; i++ ) {
                vector_type r = target->s - p.s;
                p.a = acceleration( r );
                simple_integration( p.s, p.v, p.a, 4 );
                pathOfOrbit[i] = p.s;
            }

            glDisable( GL_TEXTURE_2D );
            glVertexPointer( 2, GL_FLOAT, 0, pathOfOrbit );
            glDrawArrays( GL_LINES, 0, NUM_PREDICTIONS );
        }

    }
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
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
    return 5;
}

Orbital::value_type Orbital::radius() const
{
    return RADIUS;
}

Orbital::value_type Orbital::mass() const
{
    return 10;
}

Color Orbital::color()
{
    Color c( .4f, .4f, 1.0, 1.0f );
    return c * colorIntensity;
}

void CircleActor::collide_with( CircleActor& collider )
{
    deleteMe = true;;
}

Twister::Twister( const Orbital::vector_type& pos, const Orbital::vector_type& v )
    : Orbital( pos, v )
{
    angleAcc = 0;
    angleVel = random( 0.1f, 0.9f );
    angle = random_angle() * (180/3.145);
}

void Twister::on_off_screen()
{
    vector_type v0 = v;
    Orbital::on_off_screen();

    if( v0 != v )
        angleVel = -angleVel;
}

Twister::vector_type Twister::acceleration( const vector_type& r )
{
    return magnitude( r, target->mass() / (r*r) );
}

void Twister::move( int dt )
{
    Orbital::move( dt );

    const float ROTATION_MULTIPLIER =  1.0f / 1.0f;
    angleAcc = cross(a,v) * ROTATION_MULTIPLIER; 
    angleVel += angleAcc * dt;
    angle +=  angleVel*dt + angleAcc*dt*dt;
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
    Color c( 1.0f, 0.0f, 0.0f, 1.0f );
    return c * colorIntensity;
}

int Twister::score_value()
{
    return 10;
}
