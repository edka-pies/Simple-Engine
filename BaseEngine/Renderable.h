#pragma once
#include <glm/glm/glm.hpp>

class Shader;

class Renderable
{
public:
    virtual ~Renderable() {}
    virtual void Render(Shader& aShader, const glm::mat4& viewProjectionMatrix) = 0;
    virtual void Init() = 0;
    virtual void CleanUp() = 0;
};

