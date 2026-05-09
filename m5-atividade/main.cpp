
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "gl_utils.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define GL_LOG_FILE "gl.log"
#include <iostream>
#include <vector>
#include "Layer.h"

using namespace std;

int g_gl_width = 480;
int g_gl_height = 480;
GLFWwindow *g_window = NULL;

float PARALLAX_RATE = 0.01f;

int loadTexture(unsigned int &texture, const char *filename, GLint wrap_mode)
{
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  if (GLEW_EXT_texture_filter_anisotropic)
  {
    GLfloat max_aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    if (max_aniso >= 1.0f)
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    }
  }

  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
  if (!data)
  {
    cout << "Failed to load texture: " << filename << endl;
    return 0;
  }

  GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
  return 1;
}

int main()
{
  restart_gl_log();
  start_gl();

  vector<Layer *> layers;
  const char *backgrounds[] = {"w0.png", "w1.png", "w2.png", "w3.png", "w4.png"};
  float zs[] = {-0.54f, -0.53f, -0.52f, -0.51f, -0.50f};
  float rates[] = {0.0f, 0.2f, 0.4f, 0.6f, 0.8f};

  for (int i = 0; i < 5; i++)
  {
    Layer *layer = new Layer;
    layer->filename = backgrounds[i];
    layer->z = zs[i];
    layer->offsetx = 0.0f;
    layer->offsety = 0.0f;
    layer->ratex = rates[i];
    layer->ratey = 0.0f;
    if (!loadTexture(layer->tid, layer->filename, GL_REPEAT))
    {
      return 1;
    }
    layers.push_back(layer);
  }

  unsigned int spriteTexture;
  if (!loadTexture(spriteTexture, "sully.png", GL_CLAMP_TO_EDGE))
  {
    return 1;
  }

  float bgVertices[] = {
      1.0f,
      0.727f,
      1.0f,
      1.0f,
      1.0f,
      -0.727f,
      1.0f,
      0.0f,
      -1.0f,
      -0.727f,
      0.0f,
      0.0f,
      -1.0f,
      0.727f,
      0.0f,
      1.0f,
  };

  float spriteVertices[] = {
      0.22f,
      -0.12f,
      0.25f,
      0.25f,
      0.22f,
      -0.70f,
      0.25f,
      0.00f,
      -0.22f,
      -0.70f,
      0.00f,
      0.00f,
      -0.22f,
      -0.12f,
      0.00f,
      0.25f,
  };

  unsigned int indices[] = {
      2,
      1,
      0,
      0,
      3,
      2,
  };

  unsigned int bgVAO, bgVBO, bgEBO;
  glGenVertexArrays(1, &bgVAO);
  glGenBuffers(1, &bgVBO);
  glGenBuffers(1, &bgEBO);
  glBindVertexArray(bgVAO);
  glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int spriteVAO, spriteVBO, spriteEBO;
  glGenVertexArrays(1, &spriteVAO);
  glGenBuffers(1, &spriteVBO);
  glGenBuffers(1, &spriteEBO);
  glBindVertexArray(spriteVAO);
  glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(spriteVertices), spriteVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  GLuint bg_programme = create_programme_from_files("_camadas_vs.glsl", "_camadas_fs.glsl");
  GLuint sprite_programme = create_programme_from_files("_sprites_vs.glsl", "_sprites_fs.glsl");

  float fw = 0.25f;
  float fh = 0.25f;
  float spriteOffsetX = 0.0f;
  float spriteOffsetY = 3.0f * fh;
  int frameAtual = 0;
  double previousFrame = glfwGetTime();
  double previousMotionUpdate = glfwGetTime();
  float spritePosX = -0.75f;
  float spritePosY = 0.0f;
  float spriteSpeed = 0.45f;
  int spriteDirection = 1;
  const float spriteMinX = -0.75f;
  const float spriteMaxX = 0.75f;
  int spriteInsertIndex = (int)layers.size() / 2;
  bool upWasPressed = false;
  bool downWasPressed = false;

  auto logCharacterLayer = [&](int index)
  {
    if (index <= 0)
    {
      cout << "Personagem: atras de todas as camadas (indice " << index << ")" << endl;
    }
    else if (index >= (int)layers.size())
    {
      cout << "Personagem: na frente de todas as camadas (indice " << index << ")" << endl;
    }
    else
    {
      cout << "Personagem: entre as camadas " << (index - 1) << " e " << index
           << " (indice " << index << ")" << endl;
    }
  };

  logCharacterLayer(spriteInsertIndex);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  while (!glfwWindowShouldClose(g_window))
  {
    _update_fps_counter(g_window);
    double current_seconds = glfwGetTime();
    double dt = current_seconds - previousMotionUpdate;
    previousMotionUpdate = current_seconds;

    spritePosX += spriteSpeed * spriteDirection * (float)dt;
    if (spritePosX > spriteMaxX)
    {
      spritePosX = spriteMaxX;
      spriteDirection = -1;
    }
    else if (spritePosX < spriteMinX)
    {
      spritePosX = spriteMinX;
      spriteDirection = 1;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, g_gl_width, g_gl_height);

    glUseProgram(bg_programme);
    glBindVertexArray(bgVAO);
    for (int i = 0; i < spriteInsertIndex; i++)
    {
      layers[i]->offsetx += layers[i]->ratex * PARALLAX_RATE;
      glUniform1f(glGetUniformLocation(bg_programme, "offsetx"), layers[i]->offsetx);
      glUniform1f(glGetUniformLocation(bg_programme, "offsety"), layers[i]->offsety);
      glUniform1f(glGetUniformLocation(bg_programme, "layer_z"), layers[i]->z);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, layers[i]->tid);
      glUniform1i(glGetUniformLocation(bg_programme, "sprite"), 0);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    if ((current_seconds - previousFrame) > 0.16)
    {
      previousFrame = current_seconds;
      frameAtual = (frameAtual + 1) % 4;
      spriteOffsetX = fw * (float)frameAtual;
    }

    glUseProgram(sprite_programme);
    glBindVertexArray(spriteVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spriteTexture);
    glUniform1i(glGetUniformLocation(sprite_programme, "sprite"), 0);
    glUniform1f(glGetUniformLocation(sprite_programme, "offsetx"), spriteOffsetX);
    glUniform1f(glGetUniformLocation(sprite_programme, "offsety"), spriteOffsetY);
    glUniform2f(glGetUniformLocation(sprite_programme, "sprite_pos"), spritePosX, spritePosY);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glUseProgram(bg_programme);
    glBindVertexArray(bgVAO);
    for (int i = spriteInsertIndex; i < (int)layers.size(); i++)
    {
      layers[i]->offsetx += layers[i]->ratex * PARALLAX_RATE;
      glUniform1f(glGetUniformLocation(bg_programme, "offsetx"), layers[i]->offsetx);
      glUniform1f(glGetUniformLocation(bg_programme, "offsety"), layers[i]->offsety);
      glUniform1f(glGetUniformLocation(bg_programme, "layer_z"), layers[i]->z);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, layers[i]->tid);
      glUniform1i(glGetUniformLocation(bg_programme, "sprite"), 0);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glfwPollEvents();
    if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE))
    {
      glfwSetWindowShouldClose(g_window, 1);
    }

    bool upPressed = (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_UP));
    bool downPressed = (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_DOWN));
    int previousLayerIndex = spriteInsertIndex;

    if (upPressed && !upWasPressed)
    {
      spriteInsertIndex++;
    }
    if (downPressed && !downWasPressed)
    {
      spriteInsertIndex--;
    }

    upWasPressed = upPressed;
    downWasPressed = downPressed;

    if (spriteInsertIndex < 0)
    {
      spriteInsertIndex = 0;
    }
    if (spriteInsertIndex > (int)layers.size())
    {
      spriteInsertIndex = (int)layers.size();
    }

    if (spriteInsertIndex != previousLayerIndex)
    {
      logCharacterLayer(spriteInsertIndex);
    }

    glfwSwapBuffers(g_window);
  }

  for (int i = 0; i < (int)layers.size(); i++)
  {
    delete layers[i];
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
