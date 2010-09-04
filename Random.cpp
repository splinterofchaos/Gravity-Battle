
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
        
    return rand() % (max-min) + min;
}

float random( float min, float max )
{
    const int PRECISION = 10000;
    int val = random( (int)min*PRECISION, (int)max*PRECISION );
    return (float) val / PRECISION;
}
