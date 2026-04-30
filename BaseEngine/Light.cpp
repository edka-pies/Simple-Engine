#include "Light.h"
#include <glm/glm/gtc/constants.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

Light::Light(const LightCreateInfo* info)
{
    if (info) 
    {
        position = info->position;
        color = info->color;
        strenght = info->strenght;

        type = info->type;
        direction = info->direction;
        cutoff = info->cutoff;
        outerCutoff = info->outerCutoff;
        attenuation = info->attenuation;
    } 
    else //Just in case
    {
        position = glm::vec3(0.0f, 10.0f, 0.0f);
        color = glm::vec3(1.0f);
        strenght = 1.0f;

        type = LightType::Point;
        direction = glm::vec3(0.0f, -1.0f, 0.0f);
        cutoff = 12.5f;
        outerCutoff = 17.5f;
        attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
    }
}
