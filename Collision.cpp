
#include "Collision.h"

bool collision( CollisionData& x, CollisionData& y )
{
    return x.collide_with( y );
}

bool collision( LoopCollisionData& loop, PointCollisionData& point )
{
    Vector<float,2> diff = point.pos() - loop.pos();
    return magnitude(diff) > loop.inner_radius();
}
