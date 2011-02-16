
#pragma once

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
    static const value_type DEFULAT_MASS = 38;

    typedef std::tr1::weak_ptr<Player>   WeakPlayerPtr;
    typedef std::tr1::shared_ptr<Player> SharedPlayerPtr;

    static WeakPlayerPtr original; // A pointer to self.
    static WeakPlayerPtr copy;

    static Texture shield;
    static Texture body;

    bool invinsible;

    Player( const vector_type& position );

    void move( float dt );
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

    void move( float dt );

    void collide_with( CircleActor& collider );

    Color color();
};
