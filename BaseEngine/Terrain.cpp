#include "Terrain.h"
#include "Shader.h"
#include "MathUtils.h"
#include <glad/glad/glad.h>
#include <cmath>
#include <iostream>

Terrain::Terrain(int width, int depth, float scale)
    : width(width), depth(depth), scale(scale), VAO(0), VBO(0), EBO(0)
{
    heights.resize(width * depth);
    GenerateMesh();
    SetupBuffers();
}

Terrain::~Terrain() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Terrain::GenerateMesh() {
    vertices.clear();
    indices.clear();

    // Calculate heights 
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            float worldX = x * scale;
            float worldZ = z * scale;
            heights[z * width + x] = SmoothNoise(worldX * 0.1f, worldZ * 0.1f) * 5.0f;
        }
    }

    // Build the Vertex Buffer with Normals
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            // Position
            float worldX = x * scale;
            float worldY = heights[z * width + x];
            float worldZ = z * scale;

            // Normal Calculation (Central Difference)
            float hL = (x > 0) ? heights[z * width + (x - 1)] : worldY;
            float hR = (x < width - 1) ? heights[z * width + (x + 1)] : worldY;
            float hD = (z > 0) ? heights[(z - 1) * width + x] : worldY;
            float hU = (z < depth - 1) ? heights[(z + 1) * width + x] : worldY;

            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f * scale, hD - hU));

            // Push to vector
            vertices.push_back(worldX); 
            vertices.push_back(worldY); 
            vertices.push_back(worldZ); 
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            vertices.push_back((float)x / width); //u
            vertices.push_back((float)z / depth); //v
        }
    }

    // Generate Indices (Connecting vertices into triangles)
    for (int z = 0; z < depth - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            int topLeft = (z * width) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * width) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

void Terrain::SetupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // UV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Terrain::Render(Shader& shader, const glm::mat4& viewProj) {
    glm::mat4 model = glm::mat4(1.0f);

    shader.SetMatrix(model, "modelMatrix");

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

float Terrain::GetTriangleHeightAt(float x, float z) const {
    float gridX = x / scale;
    float gridZ = z / scale;

    if (gridX < 0 || gridX >= width - 1 || gridZ < 0 || gridZ >= depth - 1) {
        return 0.0f;
    }

    int intX = (int)gridX;
    int intZ = (int)gridZ;

    float fractX = gridX - intX;
    float fractZ = gridZ - intZ;

    float hTL = heights[intZ * width + intX];             // Top Left (0,0)
    float hTR = heights[intZ * width + (intX + 1)];       // Top Right (1,0)
    float hBL = heights[(intZ + 1) * width + intX];       // Bottom Left (0,1)
    float hBR = heights[(intZ + 1) * width + (intX + 1)]; // Bottom Right (1,1)

    float height;

    // Check which triangle we are in by looking at the diagonal
    if (fractX < (1.0f - fractZ)) {
        // Barycentric interpolation 1
        height = hTL + (hTR - hTL) * fractX + (hBL - hTL) * fractZ;
    }
    else {
        // Barycentric interpolation 2
        height = hBR + (hBL - hBR) * (1.0f - fractX) + (hTR - hBR) * (1.0f - fractZ);
    }

    return height;
}

float Terrain::GetHeightAt(float worldX, float worldZ) {
    // 1. Calculate where the player is relative to the bottom-left corner of the terrain
    // (Adjust terrainStartX and terrainStartZ based on where your terrain is spawned!)
    float terrainStartX = 0.0f;
    float terrainStartZ = 0.0f;
    float gridSize = 1.0f; // The distance between your vertices (scale)

    float terrainX = worldX - terrainStartX;
    float terrainZ = worldZ - terrainStartZ;

    // 2. Calculate which grid square (cell) the player is standing in
    int gridX = static_cast<int>(std::floor(terrainX / gridSize));
    int gridZ = static_cast<int>(std::floor(terrainZ / gridSize));

    // 3. Prevent crashing if the player walks off the edge of the map
    // (Replace maxGridX and maxGridZ with your actual terrain vertex width/height)
    int maxGridX = 100;
    int maxGridZ = 100;
    if (gridX < 0 || gridX >= maxGridX - 1 || gridZ < 0 || gridZ >= maxGridZ - 1) {
        return 0.0f; // Fallback height if they are off the map
    }

    // 4. Find where we are inside this specific grid square (values from 0.0 to 1.0)
    float xCoord = std::fmod(terrainX, gridSize) / gridSize;
    float zCoord = std::fmod(terrainZ, gridSize) / gridSize;

    // 5. Get the 4 vertex heights of this specific grid square
    // (You will need to pull these from your Terrain's vertex array/heightmap data)
    float hTopLeft = GetTriangleHeightAt(gridX, gridZ);
    float hTopRight = GetTriangleHeightAt(gridX + 1, gridZ);
    float hBottomLeft = GetTriangleHeightAt(gridX, gridZ + 1);
    float hBottomRight = GetTriangleHeightAt(gridX + 1, gridZ + 1);

    // 6. Every grid square is made of 2 triangles. Which one are we standing on?
    float answer;
    if (xCoord <= (1.0f - zCoord)) {
        // We are on the Top-Left triangle
        answer = BarryCentric(
            glm::vec3(0.0f, hTopLeft, 0.0f),
            glm::vec3(1.0f, hTopRight, 0.0f),
            glm::vec3(0.0f, hBottomLeft, 1.0f),
            glm::vec2(xCoord, zCoord)
        );
    }
    else {
        // We are on the Bottom-Right triangle
        answer = BarryCentric(
            glm::vec3(1.0f, hTopRight, 0.0f),
            glm::vec3(1.0f, hBottomRight, 1.0f),
            glm::vec3(0.0f, hBottomLeft, 1.0f),
            glm::vec2(xCoord, zCoord)
        );
    }

    return answer;
}