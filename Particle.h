
#include "Actor.h"
#include "Color.h"

class Particle : public Actor
{
  public:
    Color c;

    float scale;
    value_type maxSpeed;

    Particle( const vector_type& pos, const vector_type& v, value_type minSpeed,
              value_type maxSpeed, float scale, const Color& c );

    void draw();
};
