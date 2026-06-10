#ifndef DiamondView_h
#define DiamondView_h

#include "TilemapView.h"
#include <iostream>
using namespace std;

class DiamondView : public TilemapView
{
public:
    void computeDrawPosition(const int col, const int row, const float tw, const float th, float &targetx, float &targety) const
    {
        targetx = col * tw / 2 + row * tw / 2;
        targety = col * th / 2 - row * th / 2;
    }

    void computeMouseMap(int &col, int &row, const float tw, const float th, const float mx, const float my) const
    {
        float halfTw = tw / 2.0f;
        float halfTh = th / 2.0f;

        float c = mx / halfTw + my / halfTh;
        float r = mx / halfTw - my / halfTh;

        col = floorf(c / 2.0f);
        row = floorf(r / 2.0f);
    }

    void computeTileWalking(int &col, int &row, const int direction) const
    {
        switch (direction)
        {
        case DIRECTION_NORTH:
            col -= 1;
            row += 1;
            break;
        case DIRECTION_SOUTH:
            col += 1;
            row -= 1;
            break;
        case DIRECTION_EAST:
            col += 1;
            row += 1;
            break;
        case DIRECTION_WEST:
            col -= 1;
            row -= 1;
            break;
        case DIRECTION_NORTHEAST:
            row += 1;
            break;
        case DIRECTION_SOUTHEAST:
            col += 1;
            break;
        case DIRECTION_SOUTHWEST:
            row -= 1;
            break;
        case DIRECTION_NORTHWEST:
            col -= 1;
            break;
        }
    }
};

#endif /* DiamondView_h */
