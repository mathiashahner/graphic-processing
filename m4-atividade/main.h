#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rectangle.h"
#include "texture.h"

extern const GLint WIDTH;
extern const GLint HEIGHT;

extern const char *vertex_shader;
extern const char *fragment_shader;

extern GLuint shader;
extern GLuint vao;
extern GLFWwindow *window;
extern std::vector<Rectangle> sceneObjects;
extern std::vector<GLuint> loadedTextures;

extern int activeFilter;
extern int activeStickerTexture;
extern int selectedStickerIndex;
extern int nextLayer;

void initWindow();
void initShader();
void initTextures();
void cleanupResources();
void handleEvents();
void renderLoop();
void restartGame();
void updateCaption();
void placeSticker(float xGL, float yGL);

#endif
