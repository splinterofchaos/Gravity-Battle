
#include "Actor.h"

class Particle : public Actor
{
  public:
    struct Color
    {
        float r, g, b, a;
    } c;

    Particle( const vector_type& pos, const vector_type& v, value_type minSpeed,
              value_type maxSpeed, const Color& c );

    void draw();
};
