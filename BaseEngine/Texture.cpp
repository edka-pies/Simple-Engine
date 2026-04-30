#include <Glad/Glad/glad.h>
#include "Texture.h"
#include <iostream>
#include <StbImage/stb_image.h>

Texture::Texture() : width(0), height(0), textureObject(0)
{
}

Texture::~Texture()
{
    if (textureObject != 0)
    {
        glDeleteTextures(1, &textureObject);
	}
}

void Texture::Init(const char* aPath)
{
    int channels = 0;
    width = 0;
    height = 0;

    unsigned char* data = stbi_load(aPath, &width, &height, &channels, 0);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (data)
    {
        GLint format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Could not load texture" << std::endl;
    }

    stbi_image_free(data);
}

void Texture::Bind(unsigned int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureObject);
}

void Texture::UnBind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFiltering(GLint minFilter) {
    glBindTexture(GL_TEXTURE_2D, textureObject);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
}
