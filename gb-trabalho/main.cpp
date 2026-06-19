#define STB_IMAGE_IMPLEMENTATION
#define GL_LOG_FILE "gl.log"
#define DEFAULT_TITLE_MESSAGE "Coloque o lixo na lixeira, cuidado para não cair no rio"
#define WIN_TITLE_MESSAGE "Parabéns, você ajudou o lixeiro!"
#define LOSE_TITLE_MESSAGE "Você poluiu o rio, que vacilo..."

#include "stb_image.h"
#include "gl_utils.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "TileMap.h"
#include "DiamondView.h"
#include "ltMath.h"
#include <fstream>

using namespace std;

int g_gl_width = 950;
int g_gl_height = 950;
float xi = -1.0f;
float xf = 1.0f;
float yi = -1.0f;
float yf = 1.0f;
float w = xf - xi;
float h = yf - yi;
float tw, th, tw2, th2;
int tileSetCols = 5, tileSetRows = 5;
float tileW, tileW2;
float tileH, tileH2;
int cx = -1, cy = -1;

bool g_gameOver = false;
bool g_gameWon = false;

struct ObjectTexture
{
	GLuint id;
	float aspect;
};

struct SceneObject
{
	int col;
	int row;
	int textureIndex;
	float scale;
	float yOffset;
	bool isCar = false;
	int textureIndexBack = -1;
	int movementDirection = 1;
};

static constexpr int OBJECT_TEXTURE_COUNT = 10;
ObjectTexture objectTextures[OBJECT_TEXTURE_COUNT] = {};
vector<SceneObject> sceneObjects;

TilemapView *tview = new DiamondView();
TileMap *tmap = NULL;

GLFWwindow *g_window = NULL;

TileMap *readMap(char *filename)
{
	ifstream arq(filename);
	int w, h;
	arq >> w >> h;
	TileMap *tmap = new TileMap(w, h, 0);
	for (int r = 0; r < h; r++)
	{
		for (int c = 0; c < w; c++)
		{
			int tid;
			arq >> tid;
			cout << tid << " ";
			tmap->setTile(c, h - r - 1, tid);
		}
		cout << endl;
	}
	arq.close();
	return tmap;
}

int loadTexture(unsigned int &texture, const char *filename, int *outWidth = nullptr, int *outHeight = nullptr)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

	int width, height, nrChannels;

	unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data)
	{
		if (outWidth)
		{
			*outWidth = width;
		}
		if (outHeight)
		{
			*outHeight = height;
		}
		if (nrChannels == 4)
		{
			cout << "Alpha channel" << endl;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			cout << "Without Alpha channel" << endl;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return 0;
}

void loadObjectTextures()
{
	const char *objectFiles[OBJECT_TEXTURE_COUNT] = {
			"assets/Decor_TrashBag.png",
			"assets/Decor_TrashCan.png",
			"assets/CarType1_Back.png",
			"assets/CarType1_Front.png",
			"assets/CarType2_Back.png",
			"assets/CarType2_Front.png",
			"assets/Bus_Back.png",
			"assets/Bus_Front.png",
			"assets/Truck_Red_Back.png",
			"assets/Truck_Red_Front.png"};

	for (int i = 0; i < OBJECT_TEXTURE_COUNT; i++)
	{
		int width = 1, height = 1;
		loadTexture(objectTextures[i].id, objectFiles[i], &width, &height);
		objectTextures[i].aspect = (float)height / (float)width;
	}
}

void createObjectLayer()
{
	sceneObjects.clear();
	sceneObjects.push_back({0, 7, 0, 0.4f, -0.020f});								// player
	sceneObjects.push_back({14, 7, 1, 0.6f, -0.025f});							// trash can
	sceneObjects.push_back({2, 0, 3, 1.2f, -0.045f, true, 2});			// car 1
	sceneObjects.push_back({6, 2, 9, 1.2f, -0.045f, true, 8, -1});	// car 2
	sceneObjects.push_back({5, 7, 5, 1.2f, -0.045f, true, 4});			// car 3
	sceneObjects.push_back({11, 5, 3, 1.2f, -0.045f, true, 2, -1}); // car 4
	sceneObjects.push_back({10, 1, 7, 1.2f, -0.045f, true, 6});			// car 5
	sceneObjects.push_back({9, 14, 5, 1.2f, -0.045f, true, 4, -1}); // car 6
}

bool isWalkableTile(const int tileId)
{
	switch (tileId)
	{
	case 1:
	case 5:
	case 8:
	case 11:
	case 13:
		return true;
	default:
		return false;
	}
}

void updateTitleMessage(const char *message)
{
	char tmp[128];
	sprintf(tmp, "%s | Pressione R para reiniciar", message);
	glfwSetWindowTitle(g_window, tmp);
}

void loseGame()
{
	if (g_gameOver)
	{
		return;
	}

	g_gameOver = true;
	g_gameWon = false;
	updateTitleMessage(LOSE_TITLE_MESSAGE);
}

void winGame()
{
	if (g_gameOver)
	{
		return;
	}

	g_gameOver = true;
	g_gameWon = true;
	updateTitleMessage(WIN_TITLE_MESSAGE);
}

bool checkPlayerCarCollision()
{
	if (sceneObjects.size() < 3)
	{
		return false;
	}

	const SceneObject &player = sceneObjects[0];

	for (size_t i = 2; i < sceneObjects.size(); i++)
	{
		const SceneObject &car = sceneObjects[i];
		if (player.col == car.col && player.row == car.row)
		{
			loseGame();
			return true;
		}
	}

	return false;
}

void updateCarMovement(const double currentSeconds, const double moveCooldown)
{
	if (g_gameOver || sceneObjects.size() < 3)
	{
		return;
	}

	static double lastCarMoveTime = 0.0;
	if ((currentSeconds - lastCarMoveTime) < moveCooldown)
	{
		return;
	}

	for (size_t i = 2; i < sceneObjects.size(); i++)
	{
		SceneObject &car = sceneObjects[i];
		if (!car.isCar || car.movementDirection == 0)
		{
			continue;
		}

		car.row += car.movementDirection;
		if (car.row >= tmap->getLastRow())
		{
			car.row = tmap->getLastRow();
			car.movementDirection = -1;
		}
		else if (car.row <= 0)
		{
			car.row = 0;
			car.movementDirection = 1;
		}
	}

	lastCarMoveTime = currentSeconds;
}

bool tryMoveSceneObject(const int direction)
{
	if (g_gameOver || sceneObjects.empty() || !tmap)
	{
		return false;
	}

	int nextCol = sceneObjects[0].col;
	int nextRow = sceneObjects[0].row;

	tview->computeTileWalking(nextCol, nextRow, direction);

	if (nextCol < 0 || nextCol >= tmap->getWidth() || nextRow < 0 || nextRow >= tmap->getHeight())
	{
		return false;
	}

	sceneObjects[0].col = nextCol;
	sceneObjects[0].row = nextRow;

	if (nextCol == 14 && nextRow == 7)
	{
		winGame();
		return true;
	}

	const int tileId = tmap->getTile(nextCol, nextRow);
	if (!isWalkableTile(tileId))
	{
		loseGame();
		return true;
	}

	return true;
}

int getObjectTextureIndex(const SceneObject &obj)
{
	if (!obj.isCar)
	{
		return obj.textureIndex;
	}

	return (obj.movementDirection < 0) ? obj.textureIndexBack : obj.textureIndex;
}

int main()
{
	restart_gl_log();
	// all the GLFW and GLEW start-up code is moved to here in gl_utils.cpp
	start_gl();
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS);

	updateTitleMessage(DEFAULT_TITLE_MESSAGE);

	cout << "Tentando criar tmap" << endl;
	tmap = readMap((char *)"assets/tileset.tmap");
	tw = w / (float)tmap->getWidth();
	th = tw / 2.0f;
	tw2 = th;
	th2 = th / 2.0f;
	tileW = 1.0f / (float)tileSetCols;
	tileW2 = tileW / 2.0f;
	tileH = 1.0f / (float)tileSetRows;
	tileH2 = tileH / 2.0f;

	cout << "tw=" << tw << " th=" << th << " tw2=" << tw2 << " th2=" << th2
			 << " tileW=" << tileW << " tileH=" << tileH
			 << " tileW2=" << tileW2 << " tileH2=" << tileH2
			 << endl;

	GLuint tid;
	loadTexture(tid, "assets/tileset.png");

	tmap->setTid(tid);
	loadObjectTextures();
	createObjectLayer();
	cout << "Tmap inicializado" << endl;

	// LOAD TEXTURES

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
			// positions   // texture coords
			xi,
			yi + th2,
			0.0f,
			tileH2, // left
			xi + tw2,
			yi,
			tileW2,
			0.0f, // bottom
			xi + tw,
			yi + th2,
			tileW,
			tileH2, // right
			xi + tw2,
			yi + th,
			tileW2,
			tileH, // top
	};
	unsigned int indices[] = {
			0, 1, 3, // first triangle
			3, 1, 2	 // second triangle
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	float objectVertices[] = {
			// positions   // texture coords
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f,
	};
	unsigned int objectIndices[] = {
			0, 1, 3,
			3, 1, 2};

	unsigned int objectVBO, objectVAO, objectEBO;
	glGenVertexArrays(1, &objectVAO);
	glGenBuffers(1, &objectVBO);
	glGenBuffers(1, &objectEBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(objectVertices), objectVertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(objectIndices), objectIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	parse_file_into_str("_geral_vs.glsl", vertex_shader, 1024 * 256);
	parse_file_into_str("_geral_fs.glsl", fragment_shader, 1024 * 256);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *p = (const GLchar *)vertex_shader;
	glShaderSource(vs, 1, &p, NULL);
	glCompileShader(vs);

	// check for compile errors
	int params = -1;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
		print_shader_info_log(vs);
		return 1; // or exit or something
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	p = (const GLchar *)fragment_shader;
	glShaderSource(fs, 1, &p, NULL);
	glCompileShader(fs);

	// check for compile errors
	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
		print_shader_info_log(fs);
		return 1; // or exit or something
	}

	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: could not link shader programme GL index %i\n", shader_programme);
		return false;
	}

	double lastMoveTime = glfwGetTime();
	const double moveCooldown = 0.15;

	for (int r = 0; r < tmap->getHeight(); r++)
	{
		for (int c = 0; c < tmap->getWidth(); c++)
		{
			unsigned char t_id = tmap->getTile(c, r);
			cout << ((int)t_id) << " ";
		}
		cout << endl;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(g_window))
	{
		// _update_fps_counter(g_window);
		double current_seconds = glfwGetTime();
		checkPlayerCarCollision();
		updateCarMovement(current_seconds, 0.12);

		// wipe the drawing surface clear
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, g_gl_width, g_gl_height);
		glUseProgram(shader_programme);
		glBindVertexArray(VAO);

		float x, y;
		int r = 0, c = 0;
		for (int r = 0; r < tmap->getHeight(); r++)
		{
			for (int c = 0; c < tmap->getWidth(); c++)
			{
				int t_id = (int)tmap->getTile(c, r);
				int u = t_id % tileSetCols;
				int v = t_id / tileSetCols;

				tview->computeDrawPosition(c, r, tw, th, x, y);

				glUniform1f(glGetUniformLocation(shader_programme, "offsetx"), u * tileW);
				glUniform1f(glGetUniformLocation(shader_programme, "offsety"), v * tileH);
				glUniform1f(glGetUniformLocation(shader_programme, "tx"), x);
				glUniform1f(glGetUniformLocation(shader_programme, "ty"), y + 1.0);
				glUniform1f(glGetUniformLocation(shader_programme, "layer_z"), tmap->getZ());
				glUniform1f(glGetUniformLocation(shader_programme, "weight"), (c == cx) && (r == cy) ? 0.5 : 0.0);

				// bind Texture
				// glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tmap->getTileSet());
				glUniform1i(glGetUniformLocation(shader_programme, "sprite"), 0);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
		}

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(objectVAO);
		auto drawObject = [&](const SceneObject &obj)
		{
			tview->computeDrawPosition(obj.col, obj.row, tw, th, x, y);

			float anchorX = xi + x + tw * 0.5f;
			float anchorY = y + th * 0.5f + obj.yOffset;
			float objWidth = tw * obj.scale;
			float objHeight = objWidth * objectTextures[getObjectTextureIndex(obj)].aspect;

			float objectVerticesFrame[] = {
					anchorX - objWidth * 0.5f,
					anchorY,
					0.0f,
					0.0f,
					anchorX - objWidth * 0.5f,
					anchorY + objHeight,
					0.0f,
					1.0f,
					anchorX + objWidth * 0.5f,
					anchorY + objHeight,
					1.0f,
					1.0f,
					anchorX + objWidth * 0.5f,
					anchorY,
					1.0f,
					0.0f,
			};

			glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(objectVerticesFrame), objectVerticesFrame);

			glUniform1f(glGetUniformLocation(shader_programme, "offsetx"), 0.0f);
			glUniform1f(glGetUniformLocation(shader_programme, "offsety"), 0.0f);
			glUniform1f(glGetUniformLocation(shader_programme, "tx"), 0.0f);
			glUniform1f(glGetUniformLocation(shader_programme, "ty"), 0.0f);
			glUniform1f(glGetUniformLocation(shader_programme, "layer_z"), tmap->getZ() - 0.001f);
			glUniform1f(glGetUniformLocation(shader_programme, "weight"), 0.0f);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, objectTextures[getObjectTextureIndex(obj)].id);
			glUniform1i(glGetUniformLocation(shader_programme, "sprite"), 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		};

		for (const SceneObject &obj : sceneObjects)
		{
			drawObject(obj);
		}
		glEnable(GL_DEPTH_TEST);

		glfwPollEvents();
		if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(g_window, 1);
		}

		if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_R))
		{
			createObjectLayer();
			g_gameOver = false;
			g_gameWon = false;
			updateTitleMessage(DEFAULT_TITLE_MESSAGE);
		}

		int direction = 0;
		const bool keyUp = GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_W) || GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_UP);
		const bool keyDown = GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_S) || GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_DOWN);
		const bool keyLeft = GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_A) || GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_LEFT);
		const bool keyRight = GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_D) || GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_RIGHT);

		if (keyUp && keyRight)
		{
			direction = DIRECTION_EAST;
		}
		else if (keyDown && keyLeft)
		{
			direction = DIRECTION_WEST;
		}
		else if (keyRight && keyDown)
		{
			direction = DIRECTION_NORTH;
		}
		else if (keyUp && keyLeft)
		{
			direction = DIRECTION_SOUTH;
		}
		else if (keyUp)
		{
			direction = DIRECTION_SOUTHEAST;
		}
		else if (keyDown)
		{
			direction = DIRECTION_NORTHWEST;
		}
		else if (keyLeft)
		{
			direction = DIRECTION_SOUTHWEST;
		}
		else if (keyRight)
		{
			direction = DIRECTION_NORTHEAST;
		}

		if (direction != 0 && (current_seconds - lastMoveTime) >= moveCooldown)
		{
			tryMoveSceneObject(direction);
			lastMoveTime = current_seconds;
		}

		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(g_window);
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	delete tmap;
	return 0;
}
