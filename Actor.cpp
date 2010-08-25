
#include "Actor.h"

Actor::ActorList Actor::actors;

bool destroy_me( const Actor::ActorPointer& actor )
{
    return actor->destroyMeFlag;
}
