#include "glm/glm/glm.hpp"
float Noise(int x, int z) {
    int n = x + z * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float SmoothNoise(float x, float z) {
    float fractX = x - floor(x);
    float fractZ = z - floor(z);

    int x1 = (int)floor(x);
    int z1 = (int)floor(z);
    int x2 = x1 + 1;
    int z2 = z1 + 1;

    // Interpolate between 4 noise points
    float v1 = Noise(x1, z1);
    float v2 = Noise(x2, z1);
    float v3 = Noise(x1, z2);
    float v4 = Noise(x2, z2);

    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);

    return glm::mix(i1, i2, fractZ);
}

static float BarryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {
    float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);

    // Prevent division by zero if the triangle is degenerate
    if (det == 0.0f) return p1.y;

    float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    float l3 = 1.0f - l1 - l2;

    return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}