
#include "BarnesHutTree.h"

#include <algorithm> 

void BarnesHutTree::init( CircleActor* cp )
{
    actor = cp;

    if( actor ) {
        massPosition = cp->s * cp->mass();
        totalMass    = cp->mass();
        centerOfMass = cp->s;;
    } else {
        centerOfMass = vector( 0, 0 );
        totalMass = 0;
    }

    children[0]=children[1]=children[2]=children[3]=0;
}

BarnesHutTree::BarnesHutTree( float x, float y, float l )
    : center( x, y )
{
    length = l;
    init( 0 );
}

BarnesHutTree::BarnesHutTree( const Vector<float,2>& pos, float l, CircleActor* cp )
    : center( pos )
{
    length = l;
    init( cp ); 
}

#include <iostream>
void BarnesHutTree::insert_into_quadrant( CircleActor* cp )
{
    // Decide which quadrant to insert the actor.
    // 0 | 1
    // -----
    // 2 | 3
    static const Vector<float,2> quadVectors[4] = {
        vector( -1.f, -1.f ),
        vector(  1.f, -1.f ),
        vector(  1.f,  1.f ),
        vector( -1.f,  1.f )
    };

    unsigned int quad;

    if( cp->s.y() < center.y() )
        quad = 0;
    else
        quad = 2;

    if( cp->s.x() > center.x() )
        quad += 1;

    if( ! children[ quad ] ) {
        float newDims = length / 2;

        Vector<float,2> newCenter;
        newCenter = center + (newDims / 2) * quadVectors[quad];

        children[ quad ] = new BarnesHutTree( newCenter, newDims, cp );
    } else {
        children[ quad ]->insert( cp );
    }
}

void BarnesHutTree::insert( CircleActor* cp )
{
    if( ! cp )
        return;

    // From http://www.cs.princeton.edu/courses/archive/fall03/cs126/assignments/barnes-hut.html
    // Constructing the Barnes-Hut tree:
    //    1. If node x does not contain a body, put the new body b here.
    //    2. If node x is an internal node, update the center-of-mass and total
    //      mass of x. Recursively insert the body b in the appropriate
    //      quadrant.
    //    3. If node x is an external node, say containing a body named c, then
    //      there are two bodies b and c in the same region. Subdivide the region
    //      further by creating four children. Then, recursively insert both b
    //      and c into the appropriate quadrant(s). Since b and c may still end
    //      up in the same quadrant, there may be several subdivisions during a
    //      single insertion. Finally, update the center-of-mass and total mass
    //      of x.
    
    // Update quadrant data.
    massPosition += cp->s * cp->mass();
    totalMass    += cp->mass();
    centerOfMass = massPosition / totalMass;

    // 1:
    if( is_empty() ) {
        actor = cp;
    }

    // 2:
    else if( !is_leaf() ) {
        insert_into_quadrant( cp );
    }

    // 3: Should always be true.
    else if( actor && is_leaf() ) {
        // These recursively call insert.
        insert_into_quadrant( cp );
        insert_into_quadrant( actor );

        actor = 0;
    }
}

void BarnesHutTree::clear()
{
    for( unsigned int i=0; i < 4; i++ )
        if( children[ i ] )
            delete children[ i ];
}

Vector<float,2> BarnesHutTree::acceleration( Particle* part, AccelFunc f )
{
    Vector<float,2> accSum;
    Vector<float,2> r = centerOfMass - part->s;

    if( is_empty() )
        return accSum;

    // From http://www.cs.princeton.edu/courses/archive/fall03/cs126/assignments/barnes-hut.html
    // Calculating the force acting on a body:
    //   1. If the current node is an external node (and it is not
    //     body b), calculate the force exerted by the current node on
    //     b, and add this amount to b's net force.
    //   2. Otherwise, calculate the ratio s / d. If s / d < ?, treat
    //     this internal node as a single body, and calculate the
    //     force it exerts on body b, and add this amount to b's net
    //     force.
    //   3. Otherwise, run the procedure recursively on each of the
    //     current node's children.

    // 1. and 2.
    if( is_leaf() || length / magnitude(r) < 0.5 ) {
        accSum = f( r, totalMass );
    }
 
    // 3
    else {
        for( int i=0; i < 4; i++ )
            if( children[ i ] )
                accSum += children[i]->acceleration( part, f );
    }

    return accSum;
}

bool BarnesHutTree::is_leaf()
{
    return !(children[0]||children[1]||children[2]||children[3]);
}

bool BarnesHutTree::is_empty()
{
    return !actor && is_leaf();
}

BarnesHutTree::~BarnesHutTree()
{
    clear();
}
