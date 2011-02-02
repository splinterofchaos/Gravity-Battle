
#pragma once

struct Arena 
{
    static float minX;
    static float maxX;
    static float minY;
    static float maxY;
    static float scale;

    static float width();
    static float height();

    static float center_x();
    static float center_y();
};
