
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

    static const int ACTIVATION_DELAY = 2000;
    int activationDelay;

    Orbital( const vector_type& position, const vector_type& v );

    void on_off_screen();

    void move( int dt );
    void draw();

    int score_value();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );
};

class Twister : public Orbital
{
    float angle;

  public:
    Twister( const vector_type& pos, const vector_type& v );

    void move( int dt );
    void draw();
};
