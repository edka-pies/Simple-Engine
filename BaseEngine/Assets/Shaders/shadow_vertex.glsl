#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

void main()
{
    // The "must write to gl_Position" fix:
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
}