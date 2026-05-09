#include "main.h"
#include "event.h"

#include <algorithm>

const GLint WIDTH = 1280;
const GLint HEIGHT = 720;

const char *vertex_shader =
    "#version 410\n"
    "layout(location = 0) in vec3 vPosition;"
    "layout(location = 1) in vec2 vTexCoord;"
    "uniform mat4 matrix;"
    "out vec2 TexCoord;"
    "void main() {"
    "    gl_Position = matrix * vec4(vPosition, 1.0);"
    "    TexCoord = vTexCoord;"
    "}";

const char *fragment_shader =
    "#version 410\n"
    "in vec2 TexCoord;"
    "uniform sampler2D uTexture;"
    "uniform int uFilter;"
    "uniform int uFlipX;"
    "uniform int uFlipY;"
    "uniform int uUseChromaKey;"
    "out vec4 frag_color;"
    "void main(){"
    "  vec2 uv = TexCoord;"
    "  if (uFlipX == 1) uv.x = 1.0 - uv.x;"
    "  if (uFlipY == 1) uv.y = 1.0 - uv.y;"
    "  vec4 color = texture(uTexture, uv);"
    "  if (uUseChromaKey == 1 && color.r > 0.96 && color.g > 0.96 && color.b > 0.96) discard;"
    "  if (uFilter == 1) {"
    "    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));"
    "    color = vec4(gray, gray, gray, color.a);"
    "  } else if (uFilter == 2) {"
    "    vec3 sepia = vec3(dot(color.rgb, vec3(0.393, 0.769, 0.189)),"
    "                      dot(color.rgb, vec3(0.349, 0.686, 0.168)),"
    "                      dot(color.rgb, vec3(0.272, 0.534, 0.131)));"
    "    color = vec4(min(sepia, vec3(1.0)), color.a);"
    "  } else if (uFilter == 3) {"
    "    color = vec4(vec3(1.0) - color.rgb, color.a);"
    "  }"
    "  frag_color = color;"
    "}";

GLuint vao;
GLuint shader;
GLFWwindow *window;
std::vector<Rectangle> sceneObjects;
std::vector<GLuint> loadedTextures;
int activeFilter = 0;
int activeStickerTexture = 0;
int selectedStickerIndex = -1;
int nextLayer = 1;

namespace
{
    GLuint compileShader(GLenum type, const char *source)
    {
        GLuint shaderId = glCreateShader(type);
        glShaderSource(shaderId, 1, &source, nullptr);
        glCompileShader(shaderId);

        GLint ok = GL_FALSE;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &ok);
        if (ok != GL_TRUE)
        {
            char log[1024] = {};
            glGetShaderInfoLog(shaderId, sizeof(log), nullptr, log);
            std::cerr << "Error on shader compilation: " << log << std::endl;
        }

        return shaderId;
    }

    bool keyJustPressed(int key)
    {
        static std::vector<int> lastState(GLFW_KEY_LAST + 1, GLFW_RELEASE);
        const int current = glfwGetKey(window, key);
        const bool pressed = (current == GLFW_PRESS && lastState[key] == GLFW_RELEASE);
        lastState[key] = current;
        return pressed;
    }

    glm::mat4 buildMatrix(const Rectangle &rect)
    {
        glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
        matrix = glm::rotate(matrix, rect.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::scale(matrix, glm::vec3(rect.width * rect.scaleX, rect.height * rect.scaleY, 1.0f));
        return matrix;
    }
}

int main()
{
    initWindow();

    if (window == nullptr)
    {
        return EXIT_FAILURE;
    }

    initShader();
    initTextures();
    restartGame();

    vao = createRectangle();

    while (!glfwWindowShouldClose(window))
    {
        handleEvents();
        renderLoop();
        updateCaption();
    }

    cleanupResources();
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

    window = glfwCreateWindow(WIDTH, HEIGHT, "", nullptr, nullptr);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void initShader()
{
    const GLuint vs = compileShader(GL_VERTEX_SHADER, vertex_shader);
    const GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragment_shader);

    shader = glCreateProgram();
    glAttachShader(shader, fs);
    glAttachShader(shader, vs);
    glLinkProgram(shader);

    GLint linked = GL_FALSE;
    glGetProgramiv(shader, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE)
    {
        char log[1024] = {};
        glGetProgramInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Error on shader program linking: " << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void initTextures()
{
    loadedTextures.clear();

    GLuint backgroundTexture = loadTexture("assets/background/background.png");
    loadedTextures.push_back(backgroundTexture);

    const char *stickerPaths[] = {
        "assets/stickers/sticker1.png",
        "assets/stickers/sticker2.png",
        "assets/stickers/sticker3.png"};

    for (const char *path : stickerPaths)
    {
        GLuint stickerTexture = loadTexture(path);
        loadedTextures.push_back(stickerTexture);
    }
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

    if (keyJustPressed(GLFW_KEY_R))
    {
        restartGame();
    }

    if (keyJustPressed(GLFW_KEY_F))
    {
        activeFilter = (activeFilter + 1) % 4;
    }

    if (keyJustPressed(GLFW_KEY_1))
    {
        activeStickerTexture = 0;
    }
    if (keyJustPressed(GLFW_KEY_2) && loadedTextures.size() > 2)
    {
        activeStickerTexture = 1;
    }
    if (keyJustPressed(GLFW_KEY_3) && loadedTextures.size() > 3)
    {
        activeStickerTexture = 2;
    }

    if (selectedStickerIndex >= 0 && selectedStickerIndex < static_cast<int>(sceneObjects.size()))
    {
        Rectangle &selected = sceneObjects[selectedStickerIndex];
        if (selected.type == RectangleType::STICKER && selected.visible)
        {
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            {
                selected.rotation += 0.03f;
            }
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            {
                selected.rotation -= 0.03f;
            }
            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            {
                selected.scaleX = std::min(3.0f, selected.scaleX + 0.01f);
                selected.scaleY = std::min(3.0f, selected.scaleY + 0.01f);
            }
            if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            {
                selected.scaleX = std::max(0.2f, selected.scaleX - 0.01f);
                selected.scaleY = std::max(0.2f, selected.scaleY - 0.01f);
            }
            if (keyJustPressed(GLFW_KEY_H))
            {
                selected.flipX = !selected.flipX;
            }
            if (keyJustPressed(GLFW_KEY_V))
            {
                selected.flipY = !selected.flipY;
            }
            if (keyJustPressed(GLFW_KEY_BACKSPACE) || keyJustPressed(GLFW_KEY_DELETE))
            {
                selected.visible = false;
                selectedStickerIndex = -1;
            }
        }
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

    for (const Rectangle &rect : sceneObjects)
    {
        if (!rect.visible)
        {
            continue;
        }

        glm::mat4 matrix = buildMatrix(rect);
        glUniformMatrix4fv(glGetUniformLocation(shader, "matrix"), 1, GL_FALSE, glm::value_ptr(matrix));
        glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);
        glUniform1i(glGetUniformLocation(shader, "uFilter"), activeFilter);
        glUniform1i(glGetUniformLocation(shader, "uFlipX"), rect.flipX ? 1 : 0);
        glUniform1i(glGetUniformLocation(shader, "uFlipY"), rect.flipY ? 1 : 0);
        glUniform1i(glGetUniformLocation(shader, "uUseChromaKey"), rect.type == RectangleType::STICKER ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rect.textureId);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glfwSwapBuffers(window);
}

void restartGame()
{
    sceneObjects.clear();
    selectedStickerIndex = -1;
    nextLayer = 1;
    activeFilter = 0;

    if (!loadedTextures.empty())
    {
        sceneObjects.push_back(createBackground(loadedTextures[0]));
    }
}

void updateCaption()
{
    std::string filterName = "normal";

    if (activeFilter == 1)
        filterName = "grayscale";
    else if (activeFilter == 2)
        filterName = "filtro de cor";
    else if (activeFilter == 3)
        filterName = "negativo";

    std::string title = "M4 - Texturas e Efeitos | Click: aplicar/selecionar | Q/E: rotação | Z/X: escala | H/V: flip | F: " + filterName + " | 1-3: sticker | Del: apagar | R: reiniciar";
    glfwSetWindowTitle(window, title.c_str());
}

void placeSticker(float xGL, float yGL)
{
    const int textureSlot = std::clamp(activeStickerTexture + 1, 1, static_cast<int>(loadedTextures.size()) - 1);
    sceneObjects.push_back(createSticker(xGL, yGL, 0.35f, loadedTextures[textureSlot], nextLayer++));
    selectedStickerIndex = static_cast<int>(sceneObjects.size()) - 1;
}

void cleanupResources()
{
    for (GLuint textureId : loadedTextures)
    {
        if (textureId != 0)
        {
            glDeleteTextures(1, &textureId);
        }
    }

    loadedTextures.clear();

    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (shader != 0)
    {
        glDeleteProgram(shader);
        shader = 0;
    }
}