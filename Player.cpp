 
#include "Player.h"

#include "Keyboard.h"
#include "Draw.h"

Texture Player::shield;
Texture Player::body;

Player::WeakPlayerPtr Player::original;
Player::WeakPlayerPtr Player::copy;

Player::Player( const Player::vector_type& position )
    : CircleActor( position ), isVisible( true )
{
    isAttractor = true;
    invinsible = false;
}

void Player::move( float dt )
{
    const value_type ACC   = 0.012;

    // TODO: The value of SPEED seems not to affect this function. Why?
    const value_type SPEED = 0.0000002;

    if( Keyboard::key_down('a') || Keyboard::key_down( Keyboard::LEFT  ) )
        a.x( a.x() - ACC );
    if( Keyboard::key_down('d') || Keyboard::key_down( Keyboard::RIGHT ) )
        a.x( a.x() + ACC );
    if( Keyboard::key_down('w') || Keyboard::key_down( Keyboard::UP    ) )
        a.y( a.y() - ACC );
    if( Keyboard::key_down('s') || Keyboard::key_down( Keyboard::DOWN  ) )
        a.y( a.y() + ACC );

    if( magnitude(a) > ACC )
        magnitude( a, ACC );

    // Used in Player::mass called by CircleActor::move.
    moreGravity = Keyboard::key_down(' ');;

    if( moreGravity )
        a *= 1.05;

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

    Color c = color();
    glColor3f( c.r(), c.g(), c.b() );

    // Shield MUST be drawn before body. Body overwrites shield.
    draw::draw( shieldVerts, 4, shield.handle(), texCoords );
    draw::draw( bodyVerts,   4, body.handle(),   texCoords );

    glLoadIdentity();

    SharedPlayerPtr copy = Player::copy.lock();
    if( this==original.lock().get() && copy ) {
        vector_type connectingLine[] = { s, copy->s };

        c = ( c + copy->color() ) / 2;
        glColor3f( c.r(), c.g(), c.b() );

        // This draws a line, but the texture makes it more faint.
        draw::draw (
            &connectingLine[0][0], 2, 
            body.handle(), texCoords, GL_LINES 
        );
    }

    glLoadIdentity();
}

int Player::score_value()
{
    return 0;
}

Player::value_type Player::radius() const 
{
    return RADIUS / (1 + 0.5*!copy.expired()) * Arena::scale;
}

Player::value_type Player::mass() const
{
    value_type g;
    if( moreGravity )
        g = 100;
    else
        g = DEFULAT_MASS;

    if( !copy.expired() )
        return g / 2;
    else
        return g;
}

void Player::collide_with( CircleActor& collider )
{
    if( !invinsible && collider.isDeadly ) {
        deleteMe = true;

        SharedPlayerPtr copy = Player::copy.lock();
        if( copy )
            copy->collide_with( *this );
    } else if( ! collider.isMovable ) {
        vector_type r = unit( s - collider.s );
        a -= 2 * ( a * r ) * r;
        a *= 5;
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

void Player2::move( float dt )
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
        SharedPlayerPtr original = Player::original.lock();
        if( !original->deleteMe )
            original->collide_with( *this );
    }
}

Color Player2::color()
{
    return Color( 1, .5, 1, 1 );
}
