#pragma once
#include <glm/glm/glm.hpp>

enum class LightType : int 
{
	Point = 0,
	Directional = 1,
	Spot = 2
};

struct LightCreateInfo 
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 color = glm::vec3(1.0f);
	float strenght = 1.0f;

	// optional for directional / spot
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

	// Spot parameters (degrees)
	float cutoff = 12.5f;        // inner cone angle
	float outerCutoff = 17.5f;   // outer cone angle

	// Attenuation (constant, linear, quadratic)
	glm::vec3 attenuation = glm::vec3(1.0f, 0.09f, 0.032f);

	LightType type = LightType::Point;
};

class Light 
{
public:
	glm::vec3 position;
	glm::vec3 color;
	float strenght;

	LightType type;
	glm::vec3 direction;
	float cutoff;        // degrees
	float outerCutoff;   // degrees
	glm::vec3 attenuation;

	Light(const LightCreateInfo* info);
};
