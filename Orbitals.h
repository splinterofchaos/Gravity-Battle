
#pragma once

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "CircleActor.h"
#include "Texture.h"

#include "Player.h"

class Orbital : public CircleActor
{
  public:
    static Texture image;
    
    static const value_type RADIUS;

    static Player* target;

    static const int ACTIVATION_DELAY = 1000;
    int activationDelay;

    Orbital( const vector_type& position, const vector_type& v );

    void on_off_screen();

    void move( int dt );
    void draw();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );
};
