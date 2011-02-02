
#include "Vector.h"

// These should be generalized, later.
#include "CircleActor.h"
#include "Particle.h"

// Based after the Barnes Hut algorithm for calculating gravitational
// acceleration on n-bodies.
class BarnesHutTree
{
    // Each node remembers its center of mass and total mass, which must be
    // updated every time a new particle is added to this node or one of its
    // children.
    Vector< float, 2 > centerOfMass, massPosition; 
    float totalMass;

    // A node cannot contain a particle if it is not a leaf.
    CircleActor* actor; 

    // Actual node data:
    Vector<float,2> center;
    BarnesHutTree* children[4];
    
    float length;

    void init( CircleActor* cp );

    void insert_into_quadrant( CircleActor* cp );

  public:
    BarnesHutTree( float x, float y, float l );
    BarnesHutTree( const Vector<float,2>& pos, float l, CircleActor* cp );

    // Modifiers:
    void insert( CircleActor* p );
    void clear(); // Erase all elements.

    typedef Vector<float,2> (*AccelFunc)( const Vector<float,2>&, float );

    Vector<float,2> acceleration( Particle* part, AccelFunc f );

    // Observers:
    bool is_leaf();
    bool is_empty();

    ~BarnesHutTree();
};
