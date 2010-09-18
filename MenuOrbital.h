
#include "Orbitals.h"

class MenuOrbital : public Orbital
{
    float time;
    value_type angle;

  public:
    MenuOrbital( const vector_type& position, const vector_type& v );

    void move( int dt );
    void draw();
};
