
#ifdef _WIN32
    #include <Windows.h>
#endif

#include "Actor.h"

class Particle : public Actor
{
  public:
    struct Color
    {
        float r, g, b, a;

		Color( float r, float g, float b, float a )
			: r(r), g(g), b(b), a(a)
		{
		}
    } c;

    float scale;

    Particle( const vector_type& pos, const vector_type& v, value_type minSpeed,
              value_type maxSpeed, float scale, const Color& c );

    void draw();
};
