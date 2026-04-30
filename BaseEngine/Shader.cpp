#include <Glad/Glad/glad.h>
#include "Shader.h"
#include <fstream>
#include <sstream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <GLFW/Include/glfw3.h>
#include <iostream>

Shader::Shader(const char* vertPath, const char* fragPath)
{
    unsigned int VertexShader = LoadVertexShader(vertPath);
    unsigned int FragmentShader = LoadFragmentShader(fragPath);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, VertexShader);
    glAttachShader(shaderProgram, FragmentShader);
    glLinkProgram(shaderProgram);

    // check link status (optional but helpful)
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
}

void Shader::Use()
{
    if (shaderProgram == 0)
    {
        std::cerr << "Shader program is not initialized." << std::endl;
        return;
    }
    glUseProgram(shaderProgram);
}

void Shader::Unuse()
{
    glUseProgram(0);
}

void Shader::SetMatrix(const glm::mat4& aMatrix, const std::string& aName)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, aName.c_str()), 1, GL_FALSE, glm::value_ptr(aMatrix));
}

void Shader::SetVec3(const glm::vec3& v, const std::string& name)
{
    glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, glm::value_ptr(v));
}

void Shader::SetVec4(const glm::vec4& v, const std::string& name)
{
    glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, glm::value_ptr(v));
}

void Shader::SetFloat(float f, const std::string& name)
{
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), f);
}

void Shader::SetInt(int i, const std::string& name)
{
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), i);
}

void Shader::SetLights(const std::vector<Light*>& lights)
{
    // send up to MAX_LIGHTS
    int count = static_cast<int>(lights.size());
    if (count > MAX_LIGHTS) count = MAX_LIGHTS;
    SetInt(count, "numLights");

    for (int i = 0; i < count; ++i)
    {
        const Light* L = lights[i];
        if (!L) continue;

        // compose uniform names like "light_position[0]"
        std::string idx = "[" + std::to_string(i) + "]";

        SetVec3(L->position, "light_position" + idx);
        SetVec4(glm::vec4(L->color * L->strenght, 1.0f), "light_diffuse" + idx);
        // Ambient — small fraction of diffuse (or compute on GPU). We send a scaled ambient here.
        SetVec4(glm::vec4(L->color * 0.15f * L->strenght, 1.0f), "light_ambient" + idx);
        SetVec4(glm::vec4(L->color, 1.0f), "light_specular" + idx);

        SetVec3(L->attenuation, "light_attenuation" + idx);
        SetVec3(L->direction, "light_direction" + idx);

        // send precomputed cos of cutoffs (so shader won't call cos per fragment)
        float cosCut = glm::cos(glm::radians(L->cutoff));
        float cosOuter = glm::cos(glm::radians(L->outerCutoff));
        SetFloat(cosCut, "light_cutoff" + idx);
        SetFloat(cosOuter, "light_outerCutoff" + idx);

        SetInt(static_cast<int>(L->type), "light_type" + idx);
    }
}
 
std::string Shader::LoadFile(const char* aPath)
{
    std::ifstream file(aPath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << aPath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::LoadVertexShader(const char* aPath)
{
    int result;
    char Log[512];

    std::string shaderCodeString = LoadFile(aPath);
    const char* shaderCode = shaderCodeString.c_str();

    unsigned int shaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderObject, 1, &shaderCode, NULL);
    glCompileShader(shaderObject);

    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(shaderObject, 512, NULL, Log);
        std::cout << "Vertex shader compile error:\n" << Log << std::endl;
    }

    return shaderObject;
}

unsigned int Shader::LoadFragmentShader(const char* aPath)
{
    int result;
    char Log[512];

    std::string shaderCodeString = LoadFile(aPath);
    const char* shaderCode = shaderCodeString.c_str();

    unsigned int shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderObject, 1, &shaderCode, NULL);
    glCompileShader(shaderObject);

    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(shaderObject, 512, NULL, Log);
        std::cout << "Fragment shader compile error:\n" << Log << std::endl;
    }

    return shaderObject;
}