
#pragma once

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "Actor.h"
#include "Arena.h"

struct CircleActor : public Actor
{
    bool isActive;

    CircleActor();
    CircleActor( const vector_type& position );

    virtual value_type radius() const = 0;
    virtual value_type mass()   const = 0;

    virtual void on_off_screen();

    // Overloads Actor::move to add functionality like staying on the
    // screen.
    void move( int dt );

    // How many points are awarded for killing this. Allows negative values.
    virtual int score_value() = 0;

    void collide_with( CircleActor& collider );
};

bool collision( const CircleActor& c1, const CircleActor& c2 );
