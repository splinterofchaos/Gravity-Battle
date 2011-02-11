
#include "Keyboard.h"

#include <algorithm> 

Uint8* Keyboard::keys = 0;
Keyboard::KeyStatus Keyboard::states[ SDLK_LAST ];

void Keyboard::update()
{
    keys = SDL_GetKeyState( 0 );

    std::fill( states, states+SDLK_LAST, NOT_PRESSED );
}

void Keyboard::add_key_status( unsigned int key, KeyStatus status )
{
    states[ key ] = status;
}

bool Keyboard::key_down( unsigned int key )
{
    return keys[ key ];
}

Keyboard::KeyStatus Keyboard::key_state( unsigned int key )
{
    return states[ key ];
}
