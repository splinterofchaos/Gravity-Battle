
#include "Orbitals.h"

class Obsticle;
class Goal;

struct Package : public Orbital
{
    static const float RADIUS_TO_START = 200;
    static Goal* goal;

    bool started;
    bool reachedGoal;

    Package( const vector_type& pos, const vector_type& v );

    void move( int dt );
    void draw();

    Color color();

    void collide_with( CircleActor& collider );
};

struct Obsticle : public Orbital
{
    static const float SIZE;

    Obsticle( const vector_type& pos, const vector_type& v=vector_type(0,0) );

    Color color();

    value_type radius() const;

    void collide_with( CircleActor& collider );
};

struct Goal : public Orbital
{
    static const float SIZE;

    Goal( const vector_type& pos, const vector_type& v=vector_type(0,0) );

    Color color();

    value_type radius() const ;

    void collide_with( CircleActor& collider );
};
