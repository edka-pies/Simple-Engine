#version 410 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 eyePosition; // world-space eye position

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

out vec3 v_normal;
out vec2 UV_Coord;
out vec3 position;
out vec3 vecToEye;

void main()
{
    mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
    gl_Position = mvp * vec4(inPosition, 1.0);

    // world-space position
    position = (modelMatrix * vec4(inPosition, 1.0)).xyz;

    // Transform normal to world space. Use inverse-transpose when model has non-uniform scale.
    v_normal = normalize(mat3(transpose(inverse(modelMatrix))) * inNormal);

    UV_Coord = inUV;

    // vector from fragment to eye in world space
    vecToEye = normalize(eyePosition.xyz - position);
}