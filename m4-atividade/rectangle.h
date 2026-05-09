#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <vector>
#include <GL/glew.h>

enum class RectangleType
{
    BACKGROUND,
    STICKER
};

struct Rectangle
{
    float x, y;
    float width, height;
    float rotation;
    float scaleX, scaleY;
    bool flipX;
    bool flipY;
    GLuint textureId;
    int layer;
    RectangleType type;
    bool visible = true;
};

GLuint createRectangle();
Rectangle createBackground(GLuint textureId);
Rectangle createSticker(float x, float y, float size, GLuint textureId, int layer);

#endif
