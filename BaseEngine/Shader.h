#pragma once
#include <string>
#include <vector>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include "Scene.h"

class Shader
{
public:	
	Shader(const char* vertPath, const char* fragPath);

	void Use();
	void Unuse();
	void Reload();

	void SetMatrix(const glm::mat4& aMatrix, const std::string& aName);

	void SetVec3(const glm::vec3& v, const std::string& name);
	void SetVec4(const glm::vec4& v, const std::string& name);
	void SetFloat(float f, const std::string& name);
	void SetInt(int i, const std::string& name);

	// upload multiple lights (vector of pointers from Scene)
	void SetLights(const std::vector<Light*>& lights);

	std::string LoadFile(const char* aPath);
	unsigned int LoadVertexShader(const char* aPath);
	unsigned int LoadFragmentShader(const char* aPath);
private:
	unsigned int shaderProgram = 0;
	static const int MAX_LIGHTS = 8;

	std::string vertexPath;
	std::string fragmentPath;
};

