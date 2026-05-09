#include "main.h"

const GLint WIDTH = 1280;
const GLint HEIGHT = 720;

const char *vertex_shader =
    "#version 410\n"
    "layout(location = 0) in vec3 vPosition;"
    "uniform mat4 matrix;"
    "void main() {"
    "    gl_Position = matrix * vec4(vPosition, 1.0);"
    "}";

const char *fragment_shader =
    "#version 410\n"
    "uniform vec3 uColor;"
    "out vec4 frag_color;"
    "void main(){"
    "  frag_color = vec4(uColor, 1.0f);"
    "}";

GLuint vao;
GLFWwindow *window;
std::vector<Rectangle> rectangles;
int shader, multiplier, score, maxScore;

int main()
{
    initWindow();

    if (window == nullptr)
    {
        return EXIT_FAILURE;
    }

    initShader();
    restartGame();

    vao = createRectangle();

    while (!glfwWindowShouldClose(window))
    {
        handleEvents();
        renderLoop();
        updateScore();
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}

void initWindow()
{
    srand((unsigned int)time(nullptr));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(WIDTH, HEIGHT, "M3 - Jogo das Cores", nullptr, nullptr);
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GL_FALSE);

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << "GLEW error: " << glewGetErrorString(err) << std::endl;
    }
}

void initShader()
{
    int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    shader = glCreateProgram();
    glAttachShader(shader, fs);
    glAttachShader(shader, vs);
    glLinkProgram(shader);
}

void handleEvents()
{
    glfwPollEvents();

    static int prevMouseState = GLFW_RELEASE;
    int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    if (mouseState == GLFW_PRESS && prevMouseState == GLFW_RELEASE)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        verifyClick(mouseX, mouseY);
    }

    prevMouseState = mouseState;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        restartGame();
    }
}

void renderLoop()
{
    glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int screenWidth, screenHeight;
    glfwGetWindowSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    glUseProgram(shader);

    for (const Rectangle &rect : rectangles)
    {
        if (!rect.visible)
        {
            continue;
        }

        glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
        matrix = glm::scale(matrix, glm::vec3(rect.width, rect.height, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "matrix"), 1, GL_FALSE, glm::value_ptr(matrix));
        glUniform3f(glGetUniformLocation(shader, "uColor"), rect.r, rect.g, rect.b);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glfwSwapBuffers(window);
}

void restartGame()
{
    score = 0;
    maxScore = std::max(maxScore, score);
    multiplier = 20;
    rectangles = createRectangleGrid();
}

void updateScore()
{
    maxScore = std::max(maxScore, score);

    std::string title = "M3 - Jogo das Cores   |   Pressione (R) para reiniciar   |   Pontuação: " + std::to_string(score) + "   |   Recorde: " + std::to_string(maxScore);
    glfwSetWindowTitle(window, title.c_str());
}