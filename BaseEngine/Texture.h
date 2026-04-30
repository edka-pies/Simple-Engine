#pragma once
#include <Glad/Glad/glad.h>
class Texture
{
public:
    Texture();
	~Texture();

    int width;
    int height;

    unsigned int textureObject;

    void Init(const char* aPath);

    void Bind(unsigned int slot = 0);

    void UnBind();

    void SetFiltering(GLint minFilter);
};

