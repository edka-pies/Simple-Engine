#include "Terrain.h"
#include "Shader.h"
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
    // 1. Generate Vertices
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {

            // Calculate actual world positions
            float worldX = x * scale;
            float worldZ = z * scale;

            // Generate some fake "hills" for testing using sine waves
            float worldY = sin(worldX * 0.1f) * 2.0f + cos(worldZ * 0.1f) * 2.0f;

            // Save to our CPU height array for physics
            heights[z * width + x] = worldY;

            // Simple UP normal for now (you can calculate real normals later)
            float nx = 0.0f, ny = 1.0f, nz = 0.0f;

            // UVs
            float u = (float)x / (float)width;
            float v = (float)z / (float)depth;

            // Add to vertex array (Position, Normal, UV)
            vertices.push_back(worldX);
            vertices.push_back(worldY);
            vertices.push_back(worldZ);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    // 2. Generate Indices (Connecting vertices into triangles)
    for (int z = 0; z < depth - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            int topLeft = (z * width) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * width) + x;
            int bottomRight = bottomLeft + 1;

            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2
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

    // Assuming your Vertex shader uses layout (location = 0) for Pos, 1 for Normal, 2 for UV
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
    // Terrain usually sits at world origin (0,0,0)
    glm::mat4 model = glm::mat4(1.0f);

    // NOTE: Replace "modelMatrix" with whatever your Shader expects!
    // Some engines use "model", some use "modelMatrix". Adjust accordingly.
    shader.SetMatrix(model, "modelMatrix");

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

float Terrain::GetHeightAt(float x, float z) const {
    // 1. Find which grid square we are in
    float gridX = x / scale;
    float gridZ = z / scale;

    // 2. Ensure we are inside the terrain bounds
    if (gridX < 0 || gridX >= width - 1 || gridZ < 0 || gridZ >= depth - 1) {
        return 0.0f; // Or some fallback height like -1000.0f to let the player fall
    }

    // 3. Get the integer coordinates of the square's corners
    int intX = (int)gridX;
    int intZ = (int)gridZ;

    // 4. Determine exactly where we are inside that specific square (values 0.0 to 1.0)
    float fractX = gridX - intX;
    float fractZ = gridZ - intZ;

    // 5. Get the height of the 4 corners of this square from our CPU array
    float h00 = heights[intZ * width + intX];             // Top Left
    float h10 = heights[intZ * width + (intX + 1)];       // Top Right
    float h01 = heights[(intZ + 1) * width + intX];       // Bottom Left
    float h11 = heights[(intZ + 1) * width + (intX + 1)]; // Bottom Right

    // 6. Bilinear Interpolation (Smoothly blend between the 4 corners based on our fractional position)
    float heightTop = h00 * (1.0f - fractX) + h10 * fractX;
    float heightBottom = h01 * (1.0f - fractX) + h11 * fractX;

    return heightTop * (1.0f - fractZ) + heightBottom * fractZ;
}