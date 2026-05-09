#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <GL/glew.h>

GLuint loadTexture(const std::string &path, bool flipVertically = true);

#endif
