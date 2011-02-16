
#include "Timer.h"

#include <SDL/SDL.h>

const int Timer::SECOND = 1000;

Timer::Timer()
{
    zero();

    // I don't know why yet, but without this line, the game will freeze on
    // start.
    lastTime = 1;
}

void Timer::zero()
{
    reset();
    lastTime = 0;
}

int Timer::update()
{
    int newTime = SDL_GetTicks();
    lastTime = newTime - startTime;

    return newTime;
}

int Timer::reset()
{
    return startTime = update();
}

void Timer::clamp_ms( float ms )
{
    if( lastTime > ms )
        lastTime = ms;
}

float Timer::time_ms()
{
    return lastTime;
}

float Timer::time_sec()
{
    return time_ms() / float(SECOND);
}
