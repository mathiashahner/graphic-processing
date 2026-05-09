#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <vector>
#include <cstdlib>
#include <GL/glew.h>

const int rows = 8, cols = 6;
const float outerMargin = 0.05f;
const float gap = 0.01f;

struct Rectangle
{
    float x, y;
    float width, height;
    float r, g, b;
    bool visible = true;
};

std::vector<Rectangle> createRectangleGrid();
GLuint createRectangle();

#endif
