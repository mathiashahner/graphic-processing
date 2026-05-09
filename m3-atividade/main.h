#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rectangle.h"
#include "event.h"

extern const GLint WIDTH;
extern const GLint HEIGHT;

extern const char *vertex_shader;
extern const char *fragment_shader;

extern int shader;
extern GLuint vao;
extern GLFWwindow *window;
extern std::vector<Rectangle> rectangles;

extern int score;
extern int multiplier;

void initWindow();
void initShader();
void handleEvents();
void renderLoop();
void restartGame();
void updateScore();

#endif
