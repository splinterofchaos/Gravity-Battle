
#pragma once

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "CircleActor.h"
#include "Texture.h"

class Player : public CircleActor
{
    bool isVisible;

  public:
    static Texture shield;
    static Texture body;

    Player( const vector_type& position );

    void move( int dt );
    void draw();

    value_type radius() const;
    value_type mass()   const;

    void collide_with( CircleActor& collider );
};
