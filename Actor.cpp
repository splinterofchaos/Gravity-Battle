
#include "Actor.h"


Actor::Actor( const vector_type& pos )
    : s(pos), previousS(s), maxSpeed(0), scale( 1 )
{
    init();
}

