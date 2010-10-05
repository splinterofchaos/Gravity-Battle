
#include "Orbitals.h"

class MenuOrbital : public Orbital
{
    float time;
    value_type angle;
    value_type speed;

  public:
    MenuOrbital( const vector_type& position, const vector_type& v );

    void move( int dt );
    void draw();

    void collide_with( CircleActor& collider );
};
