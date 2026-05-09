#include "event.h"
#include "main.h"

void verifyClick(double mouseX, double mouseY)
{
    float xGL = (static_cast<float>(mouseX) / static_cast<float>(WIDTH)) * 2.0f - 1.0f;
    float yGL = 1.0f - (static_cast<float>(mouseY) / static_cast<float>(HEIGHT)) * 2.0f;

    for (Rectangle &rect : rectangles)
    {
        float left = rect.x - (rect.width * 0.5f);
        float right = rect.x + (rect.width * 0.5f);
        float bottom = rect.y - (rect.height * 0.5f);
        float top = rect.y + (rect.height * 0.5f);

        if (xGL >= left && xGL <= right && yGL >= bottom && yGL <= top && rect.visible)
        {
            findRectanglesSimilarColor(rect);
            multiplier = std::max(1, multiplier - 1);
            break;
        }
    }
}

void findRectanglesSimilarColor(const Rectangle &clickedRect)
{
    for (Rectangle &rect : rectangles)
    {
        if (!rect.visible)
        {
            continue;
        }

        float distance = colorDistance(clickedRect.r, clickedRect.g, clickedRect.b, rect.r, rect.g, rect.b);

        if (distance < 0.00075f)
        {
            rect.visible = false;
            score += multiplier;
        }
    }
}

float colorDistance(float r1, float g1, float b1, float r2, float g2, float b2)
{
    const float dMax = 441.6729559300637f;

    const float dr = r1 - r2;
    const float dg = g1 - g2;
    const float db = b1 - b2;

    return std::sqrt((dr * dr) + (dg * dg) + (db * db)) / dMax;
}
