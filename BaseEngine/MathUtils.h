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