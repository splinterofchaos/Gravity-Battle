 
#include "Player.h"

#include <SDL/SDL.h> // For 

Texture Player::shield;
Texture Player::body;

Player * Player::original = 0;
Player2* Player::copy = 0;

Player::Player( const Player::vector_type& position )
    : CircleActor( position ), isVisible( true )
{
}

void Player::move( int dt )
{
    Uint8* keyStates = SDL_GetKeyState( 0 );

    const value_type ACC   = 0.01;

    // TODO: The value of SPEED seems not to affect this function. Why?
    const value_type SPEED = 0.0000005;

    if( keyStates[ SDLK_a ] )
        a.x( a.x() - ACC );
    if( keyStates[ SDLK_d ] )
        a.x( a.x() + ACC );
    if( keyStates[ SDLK_w ] )
        a.y( a.y() - ACC );
    if( keyStates[ SDLK_s ] )
        a.y( a.y() + ACC );

    if( magnitude(a) > ACC )
        magnitude( a, ACC );

    // Used in Player::mass called by CircleActor::move.
    moreGravity = keyStates[ SDLK_SPACE ];

    CircleActor::move( dt, SPEED );

    //v *= 0.2;
    a *= 0.9;
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
    glDisable( GL_DEPTH_TEST );

    Color c = color();
    glColor3f( c.r(), c.g(), c.b() );

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

        glLoadIdentity();

        if( this==original && copy ) {
            vector_type connectingLine[] = { s, copy->s };
            c = ( c + copy->color() ) / 2;

            glColor3f( c.r(), c.g(), c.b() );
            glVertexPointer( 2, GL_FLOAT, 0, connectingLine );
            glDrawArrays( GL_LINES, 0, 2 );
        }

    }
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );

    glEnable( GL_DEPTH_TEST );

    glLoadIdentity();
}

int Player::score_value()
{
    return 0;
}

Player::value_type Player::radius() const 
{
    return RADIUS / (1 + 0.5*!!copy) * Arena::scale;
}

Player::value_type Player::mass() const
{
    value_type g;
    if( moreGravity )
        g = 50;
    else
        g = 18;

    if( copy )
        return g / 2;
    else
        return g;
}

void Player::collide_with( CircleActor& collider )
{
    if( collider.isDeadly ) {
        deleteMe = true;

        if( copy )
            copy->collide_with( *this );
    } else {
        a = vector_type(0,0);
        v /= 10;
    }
}

Color Player::color()
{
    return Color(1,1,1,1);
}

Player2::Player2( const vector_type& pos )
    : Player( pos )
{
}

void Player2::move( int dt )
{
    Uint8* keyStates = SDL_GetKeyState( 0 );

    const value_type SPEED = 0.25;

    v = vector_type( 0, 0 );

    if( keyStates[ SDLK_LEFT ] )
        v.x( v.x() - SPEED );
    if( keyStates[ SDLK_RIGHT ] )
        v.x( v.x() + SPEED );
    if( keyStates[ SDLK_UP ] )
        v.y( v.y() - SPEED );
    if( keyStates[ SDLK_DOWN ] )
        v.y( v.y() + SPEED );

    moreGravity = keyStates[ SDLK_SPACE ];

    CircleActor::move( dt );
}

void Player2::collide_with( CircleActor& collider )
{
    if( collider.isMovable ) {
        deleteMe = true;

        // One can't survive without the other.
        if( !original->deleteMe )
            original->collide_with( *this );
    }
}

Color Player2::color()
{
    return Color( 1, .5, 1, 1 );
}
