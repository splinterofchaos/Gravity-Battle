 
#include "Orbitals.h"

Texture Orbital::image;

const Orbital::value_type Orbital::RADIUS = 18;

Player* Orbital::target = 0;

Orbital::Orbital( const Orbital::vector_type& pos, const Orbital::vector_type& vel )
    : CircleActor( pos )
{
    v = vel;
    activationDelay = ACTIVATION_DELAY;
    isActive = false;
}

void Orbital::on_off_screen()
{
    if( s.x() - radius() < 0 && v.x() < 0 )
        v.x( -v.x() );
    else if( s.x() + radius() > Arena::maxX && v.x() > 0 )
        v.x( -v.x() );

    if( s.y() - radius() < 0 && v.y() < 0 )
        v.y( -v.y() );
    else if( s.y() + radius() > Arena::maxY && v.y() > 0 )
        v.y( -v.y() );
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

    glEnable( GL_TEXTURE_2D );
    glColor4f( activationProgress, activationProgress, activationProgress, activationProgress );

    glBindTexture( GL_TEXTURE_2D, image.handle() );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    {
        glTexCoordPointer( 2, GL_INT, 0, texCoords );
        glVertexPointer( 2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_QUADS, 0, 4 );

    }
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );

    glLoadIdentity();
}

Orbital::value_type Orbital::radius() const
{
    return RADIUS;
}

Orbital::value_type Orbital::mass() const
{
    return 10;
}

void CircleActor::collide_with( CircleActor& collider )
{
    deleteMe = true;;
}
