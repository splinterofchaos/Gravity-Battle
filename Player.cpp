 
#include "Player.h"

#include <SDL/SDL.h> // For 

Texture Player::shield;
Texture Player::body;

Player::Player( const Player::vector_type& position )
    : CircleActor( position ), isVisible( true )
{
}

void Player::move( int dt )
{
    Uint8* keyStates = SDL_GetKeyState( 0 );

    const value_type SPEED = 0.25;

    v = vector_type( 0, 0 );

    if( keyStates[ SDLK_a ] )
        v.x( v.x() - SPEED );
    if( keyStates[ SDLK_d ] )
        v.x( v.x() + SPEED );
    if( keyStates[ SDLK_w ] )
        v.y( v.y() - SPEED );
    if( keyStates[ SDLK_s ] )
        v.y( v.y() + SPEED );

    CircleActor::move( dt );
}

void Player::draw()
{
    if( !isVisible )
        return;

    glTranslatef( s.x(), s.y(), 0 );

    float shieldVerts[] = { 
        -radius(), -radius(),
         radius(), -radius(),
         radius(),  radius(),        
        -radius(),  radius(),
    };

    float bodyVerts[] = { 
        -radius()/2, -radius()/2,
         radius()/2, -radius()/2,
         radius()/2,  radius()/2,        
        -radius()/2,  radius()/2,
    };

    int texCoords[] = {
        0, 0,
        1, 0,
        1, 1, 
        0, 1
    };

    glEnable( GL_TEXTURE_2D );
    glColor3f( 1, 1, 1 );

    // Transparency used for drawing body on to of shield.
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //glDisable( GL_DEPTH_TEST );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    {
        glTexCoordPointer( 2, GL_INT, 0, texCoords );

        // Draw the shield first.
        glVertexPointer( 2, GL_FLOAT, 0, shieldVerts );
        glBindTexture( GL_TEXTURE_2D, shield.handle() );
        glDrawArrays( GL_QUADS, 0, 4 );

        // Draw the body.
        glVertexPointer( 2, GL_FLOAT, 0, bodyVerts );
        glBindTexture( GL_TEXTURE_2D, body.handle() );
        glDrawArrays( GL_QUADS, 0, 4 );

    }
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );

    glLoadIdentity();
}

Player::value_type Player::radius() const 
{
    return 25;
}

Player::value_type Player::mass() const
{
    return 18;
}

void Player::collide_with( CircleActor& collider )
{
    deleteMe = true;
}
