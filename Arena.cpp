
#include "Arena.h"


float Arena::minX = 0;
float Arena::maxX = 0;
float Arena::minY = 0;
float Arena::maxY = 0;
float Arena::scale = 0.75;

float Arena::width()
{
    return maxX - minX;
}

float Arena::height()
{
    return maxY - minY;
}

float Arena::center_x()
{
    return minX + width() / 2;
}

float Arena::center_y()
{
    return minY + height() / 2;
}
