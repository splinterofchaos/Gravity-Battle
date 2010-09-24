
#pragma once

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "CircleActor.h"
#include "Texture.h"

class Player2;

class Player : public CircleActor
{
    friend class Player2;

    bool isVisible;
    bool moreGravity;

    static const float RADIUS = 25;

  public:
    static Player*  original; // A pointer to self.
    static Player2* copy;

    static Texture shield;
    static Texture body;

    Player( const vector_type& position );

    void move( int dt );
    void draw();

    int score_value();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );

    Color color();
};

struct Player2 : public Player
{

    Player2( const vector_type& pos );

    void move( int dt );

    void collide_with( CircleActor& collider );

    Color color();
};
