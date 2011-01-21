
#include "Random.h"

#include <cstdlib>
#include <ctime>

int cheat()
{
    srand( time(0) );
    return 0;
}

int cheater = cheat();

// Postcondition: min <= random(min,max) < max
int random( int min, int max )
{
    if( max-min == 0 )
        return min;

    return rand()%(max-min) + min;
}

float random( float min, float max )
{
    const float PRECISION = 1000;
    int val = random( int(min*PRECISION), int(max*PRECISION) );
    return val / PRECISION;
}

double random( double min, double max )
{
    const double PRECISION = 10000;
    int val = random( int(min*PRECISION), int(max*PRECISION) );
    return val / PRECISION;
}

double random_angle()
{
    return random(0.0,3.14159265) * 2;
}
