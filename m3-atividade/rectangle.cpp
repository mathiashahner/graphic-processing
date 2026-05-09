#include "rectangle.h"

static float randomRGB()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

std::vector<Rectangle> createRectangleGrid()
{
    std::vector<Rectangle> rectangles;

    const float availableWidth = 2.0f - (2.0f * outerMargin) - (static_cast<float>(cols - 1) * gap);
    const float availableHeight = 2.0f - (2.0f * outerMargin) - (static_cast<float>(rows - 1) * gap);
    const float cellWidth = availableWidth / static_cast<float>(cols);
    const float cellHeight = availableHeight / static_cast<float>(rows);

    const float startX = -1.0f + outerMargin + (cellWidth * 0.5f);
    const float startY = 1.0f - outerMargin - (cellHeight * 0.5f);

    rectangles.reserve(rows * cols);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            Rectangle rect;
            rect.x = startX + static_cast<float>(col) * (cellWidth + gap);
            rect.y = startY - static_cast<float>(row) * (cellHeight + gap);
            rect.width = cellWidth;
            rect.height = cellHeight;
            rect.r = randomRGB();
            rect.g = randomRGB();
            rect.b = randomRGB();
            rectangles.push_back(rect);
        }
    }

    return rectangles;
}

GLuint createRectangle()
{
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,

        -0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f};

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}
