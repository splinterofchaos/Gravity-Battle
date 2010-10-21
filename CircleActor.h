
#pragma once

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "Actor.h"
#include "Arena.h"
#include "Color.h"

class CircleActor : public Actor
{
    void init();

  protected:
    struct State {
        vector_type s, v, a;
    };

    void state( const State& state );
    State state();

  public:
    bool isActive;
    bool isMovable;
    bool isDeadly;

    CircleActor();
    CircleActor( const vector_type& position );

    virtual value_type radius() const = 0;
    virtual value_type mass()   const = 0;

    virtual State on_off_screen( State state );

    virtual State integrate( State state, int dt, value_type maxSpeed );

    // Overloads Actor::move to add functionality like staying on the
    // screen.
    void move( int dt, value_type maxSpeed=0 );

    // How many points are awarded for killing this. Allows negative values.
    virtual int score_value() = 0;

    virtual void collide_with( CircleActor& collider ) = 0;

    // The general color of this. 
    virtual Color color() = 0;
};

bool collision( const CircleActor& c1, const CircleActor& c2 );
