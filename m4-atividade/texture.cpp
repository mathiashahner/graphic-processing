#include "texture.h"

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
  GLuint uploadTexture(const unsigned char *pixels, int width, int height, GLenum sourceFormat, GLenum internalFormat)
  {
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, sourceFormat, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureId;
  }
}

GLuint loadTexture(const std::string &path, bool flipVertically)
{
  stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

  int width = 0;
  int height = 0;
  int channels = 0;
  unsigned char *pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (pixels == nullptr)
  {
    std::cerr << "Error on load texture: " << path << " | stb: " << stbi_failure_reason() << std::endl;
    return 0;
  }

  GLenum sourceFormat = GL_RGB;
  GLenum internalFormat = GL_RGB;

  if (channels == 1)
  {
    sourceFormat = GL_RED;
    internalFormat = GL_RED;
  }
  else if (channels == 3)
  {
    sourceFormat = GL_RGB;
    internalFormat = GL_RGB;
  }
  else if (channels == 4)
  {
    sourceFormat = GL_RGBA;
    internalFormat = GL_RGBA;
  }
  else
  {
    std::cerr << "Channel format not supported for texture: " << path << std::endl;
    stbi_image_free(pixels);
    return 0;
  }

  const GLuint textureId = uploadTexture(pixels, width, height, sourceFormat, internalFormat);
  stbi_image_free(pixels);
  return textureId;
}
