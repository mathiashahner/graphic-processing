#include "rectangle.h"

GLuint createRectangle()
{
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,

        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

Rectangle createBackground(GLuint textureId)
{
    Rectangle rect;
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = 2.0f;
    rect.height = 2.0f;
    rect.rotation = 0.0f;
    rect.scaleX = 1.0f;
    rect.scaleY = 1.0f;
    rect.flipX = false;
    rect.flipY = false;
    rect.textureId = textureId;
    rect.layer = 0;
    rect.type = RectangleType::BACKGROUND;
    rect.visible = true;
    return rect;
}

Rectangle createSticker(float x, float y, float size, GLuint textureId, int layer)
{
    Rectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = size;
    rect.height = size;
    rect.rotation = 0.0f;
    rect.scaleX = 1.0f;
    rect.scaleY = 1.0f;
    rect.flipX = false;
    rect.flipY = false;
    rect.textureId = textureId;
    rect.layer = layer;
    rect.type = RectangleType::STICKER;
    rect.visible = true;
    return rect;
}
