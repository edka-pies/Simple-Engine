#pragma once
#include <vector>
#include <glm/glm/glm.hpp>

class Shader; // Forward declaration

class Terrain {
public:
    // width and depth are the number of vertices. scale is the distance between vertices.
    Terrain(int width, int depth, float scale);
    ~Terrain();

    void Render(Shader& shader, const glm::mat4& viewProj);

    // THIS IS THE MAGIC METHOD FOR PHYSICS:
    // Pass in the player's X and Z, and it returns the Y height of the floor.
    float GetHeightAt(float x, float z) const;

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