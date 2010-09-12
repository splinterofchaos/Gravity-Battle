 
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

        if( true )
        { 
            // Use fake gravity.
            const float MULTIPLIER = 1.0f / 100.0f;
            a = magnitude( r, target->mass()*MULTIPLIER / magnitude(r) );
        }
        else
        {
            // F_grav = G*m1*m2 / r^2
            // a1 = G*m2 / r^2
            // Assuming target->mass() = G*m2
            a = magnitude( r, target->mass() / (r*r) / 100 );
        }
    }
    else
    {
        a = vector_type( 0, 0 );
    }

    CircleActor::move( dt );
}

void Orbital::draw()
{
    glTranslatef( s.x(), s.y(), 0 );

    GLfloat activationProgress;
    if( isActive )
        activationProgress = 1;
    else
        activationProgress = 1.0f - (float)activationDelay / ACTIVATION_DELAY;

    float verts[] = { 
        -radius()*activationProgress, -radius()*activationProgress,
         radius()*activationProgress, -radius()*activationProgress,
         radius()*activationProgress,  radius()*activationProgress,        
        -radius()*activationProgress,  radius()*activationProgress,
    };

    int texCoords[] = {
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
        glTexCoordPointer( 2, GL_INT, 0, texCoords );
        glVertexPointer( 2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_QUADS, 0, 4 );

        glLoadIdentity();

        if( target ) {
            float accelerationLine[] = {
                s.x(), s.y(),
                target->s.x(), target->s.y()
            };
            glVertexPointer( 2, GL_FLOAT, 0, accelerationLine );
            glDrawArrays( GL_LINES, 0, 2 );
        }

    }
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );

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

void Twister::move( int dt )
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

        if( true )
        { 
            // Use fake gravity.
            const float MULTIPLIER = 1.8;
            a = magnitude( r, target->mass()*MULTIPLIER / (r*r) );
        }
        else
        {
            // F_grav = G*m1*m2 / r^2
            // a1 = G*m2 / r^2
            // Assuming target->mass() = G*m2
            a = magnitude( r, target->mass() / (r*r) / 100 );
        }
    }
    else
    {
        a = vector_type( 0, 0 );
    }

    CircleActor::move( dt );

    const float ROTATION_MULTIPLIER =  1.0f / 1.0f;
    angleAcc = cross(a,v) * ROTATION_MULTIPLIER; 
    angleVel += angleAcc * dt;
    angle +=  angleVel*dt + angleAcc*dt*dt;
}

void Twister::draw()
{
    glTranslatef( s.x(), s.y(), 0 );

    GLfloat activationProgress;
    if( isActive )
        activationProgress = 1;
    else
        activationProgress = 1.0f - (float)activationDelay / ACTIVATION_DELAY;

    float verts[] = { 
        -radius()*activationProgress, -radius()*activationProgress/2,
         radius()*activationProgress, -radius()*activationProgress/2,
         radius()*activationProgress,  radius()*activationProgress/2,        
        -radius()*activationProgress,  radius()*activationProgress/2,
    };

    int texCoords[] = {
        0, 0,
        1, 0,
        1, 1, 
        0, 1
    };

    Color c = color() * activationProgress;

    glEnable( GL_TEXTURE_2D );
    glColor4f( c.r(), c.g(), c.b(), c.a() );

    glBindTexture( GL_TEXTURE_2D, 1 );

    glRotatef( angle, 0, 0, 1 );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    {
        glTexCoordPointer( 2, GL_INT, 0, texCoords );
        glVertexPointer( 2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_QUADS, 0, 4 );

        glLoadIdentity();

        if( target ) {
            float accelerationLine[] = {
                s.x(), s.y(),
                target->s.x(), target->s.y()
            };
            glVertexPointer( 2, GL_FLOAT, 0, accelerationLine );
            glDrawArrays( GL_LINES, 0, 2 );
        }
    }
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );

    glLoadIdentity();
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
