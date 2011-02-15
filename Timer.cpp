
#include "Timer.h"

#include <SDL/SDL.h>

const int Timer::SECOND = 1000;

Timer::Timer()
{
    reset();
}

void Timer::zero()
{
    update();
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

void Timer::clamp_ms( int ms )
{
    if( lastTime > ms )
        lastTime = ms;
}

int Timer::time_ms()
{
    return lastTime;
}

float Timer::time_sec()
{
    return time_ms() / float(SECOND);
}
