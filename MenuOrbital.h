
#include "Orbitals.h"

class MenuOrbital : public Orbital
{
    float time;      // Actual time.
    float stateTime; // Time used in integration.

    value_type angle, stateAngle;
    value_type speed;

  public:
    MenuOrbital( const vector_type& position, const vector_type& v );

    State integrate( State state, int dt, value_type maxSpeed=0 );
    void move( int dt );

    void draw();

    void collide_with( CircleActor& collider );

    void register_attractor( const CircleActor& attr )
    {
    }
};
