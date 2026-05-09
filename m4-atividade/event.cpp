#include "event.h"
#include "main.h"

namespace
{
    bool containsPoint(const Rectangle &rect, float xGL, float yGL)
    {
        const float dx = xGL - rect.x;
        const float dy = yGL - rect.y;

        const float cosR = std::cos(rect.rotation);
        const float sinR = std::sin(rect.rotation);

        const float localX = (cosR * dx + sinR * dy);
        const float localY = (-sinR * dx + cosR * dy);

        const float halfW = (rect.width * rect.scaleX) * 0.5f;
        const float halfH = (rect.height * rect.scaleY) * 0.5f;

        return localX >= -halfW && localX <= halfW && localY >= -halfH && localY <= halfH;
    }
}

void verifyClick(double mouseX, double mouseY)
{
    const float xGL = (static_cast<float>(mouseX) / static_cast<float>(WIDTH)) * 2.0f - 1.0f;
    const float yGL = 1.0f - (static_cast<float>(mouseY) / static_cast<float>(HEIGHT)) * 2.0f;

    const int selected = pickSticker(xGL, yGL);
    if (selected >= 0)
    {
        selectedStickerIndex = selected;
        return;
    }

    if (loadedTextures.size() > 1)
    {
        placeSticker(xGL, yGL);
    }
}

int pickSticker(float xGL, float yGL)
{
    int bestIndex = -1;
    int bestLayer = -1;

    for (int i = 0; i < static_cast<int>(sceneObjects.size()); i++)
    {
        const Rectangle &rect = sceneObjects[i];
        if (!rect.visible || rect.type != RectangleType::STICKER)
        {
            continue;
        }

        if (containsPoint(rect, xGL, yGL) && rect.layer > bestLayer)
        {
            bestLayer = rect.layer;
            bestIndex = i;
        }
    }

    return bestIndex;
}
