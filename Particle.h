
#ifdef _WIN32
    #include <Windows.h>
#endif

#include "Actor.h"
#include "Color.h"

class Particle : public Actor
{
  public:
    Color c;

    float scale;

    Particle( const vector_type& pos, const vector_type& v, value_type minSpeed,
              value_type maxSpeed, float scale, const Color& c );

    void draw();
};
