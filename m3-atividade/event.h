#ifndef EVENT_H
#define EVENT_H

#include "rectangle.h"
#include <GLFW/glfw3.h>

void verifyClick(double mouseX, double mouseY);
void findRectanglesSimilarColor(const Rectangle &clickedRect);
float colorDistance(float r1, float g1, float b1, float r2, float g2, float b2);

#endif
