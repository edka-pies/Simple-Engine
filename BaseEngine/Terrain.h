#pragma once
#include <vector>
#include <glm/glm/glm.hpp>

class Shader; 

class Terrain {
public:
    Terrain(int width, int depth, float scale);
    ~Terrain();

    void Render(Shader& shader, const glm::mat4& viewProj);

    float GetTriangleHeightAt(float x, float z) const;

	float GetHeightAt(float worldX, float worldZ);

private:
    void GenerateMesh();
    void SetupBuffers();

    int width;
    int depth;
    float scale;

    std::vector<float> heights;    // Store heights for CPU physics
    std::vector<float> vertices;   // Position, Normal, UV
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
};