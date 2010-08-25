
#include "math/Vector.h"

#pragma once

struct CollisionData;
struct LoopCollisionData;
struct PointCollisionData;

bool collision( CollisionData& x, CollisionData& y );
bool collision( LoopCollisionData& loop, PointCollisionData& point );

struct CollisionData
{
    // This helps decouple collision data from whatever uses it.
    virtual Vector<float,2>& pos() = 0;

    virtual bool collide_with( CollisionData& ) = 0;
    virtual bool collide_with( LoopCollisionData& ) = 0;
    virtual bool collide_with( PointCollisionData& ) = 0;
};

struct LoopCollisionData : CollisionData
{
    virtual float inner_radius() = 0;
    virtual float outer_radius() = 0;

    virtual bool collide_with( CollisionData& x )
    {
        return x.collide_with( *this );
    }

    virtual bool collide_with( LoopCollisionData& x )
    {
        // Thus far, loops don't collide.
        return false;
    }

    virtual bool collide_with( PointCollisionData& x )
    {
        return collision( *this, x );
    }
};

struct PointCollisionData : CollisionData
{
    virtual bool collide_with( CollisionData& x )
    {
        return x.collide_with( *this );
    }

    virtual bool collide_with( LoopCollisionData& x )
    {
        return x.collide_with( *this );
    }

    virtual bool collide_with( PointCollisionData& x )
    {
        return false; // This'd be practically impossible, anyway.
    }
};

