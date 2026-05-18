#include "Primitives.h"
#include <glm/glm/gtc/constants.hpp>

std::shared_ptr<Mesh> Primitives::BuildMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    auto mesh = std::make_shared<Mesh>();
    mesh->SetVertexData(vertices);
    mesh->SetIndexData(indices);
    mesh->vertices = vertices;
    mesh->indices = indices;

    if (!vertices.empty()) {
        mesh->localAABB = Mesh::CalculateLocalAABB(vertices);
    }
    mesh->Init();
    return mesh;
}

std::shared_ptr<Mesh> Primitives::CreateCube() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Helper macro to easily define a vertex
    auto makeVert = [](float x, float y, float z, float nx, float ny, float nz, float u, float v) {
        Vertex vert;
        vert.position = glm::vec3(x, y, z);
        vert.normal = glm::vec3(nx, ny, nz);
        vert.textureCoord = glm::vec2(u, v);
        return vert;
        };

    vertices.push_back(makeVert(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
    vertices.push_back(makeVert(-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));

    vertices.push_back(makeVert(0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
    vertices.push_back(makeVert(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
    vertices.push_back(makeVert(-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));
    vertices.push_back(makeVert(0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));

    vertices.push_back(makeVert(-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    vertices.push_back(makeVert(-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
    vertices.push_back(makeVert(-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
    vertices.push_back(makeVert(-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));

    vertices.push_back(makeVert(0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
    vertices.push_back(makeVert(0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));

    vertices.push_back(makeVert(-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f));
    vertices.push_back(makeVert(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f));

    vertices.push_back(makeVert(-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f));
    vertices.push_back(makeVert(0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f));
    vertices.push_back(makeVert(-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f));

    for (int i = 0; i < 6; ++i) {
        int offset = i * 4;
        indices.push_back(offset + 0); indices.push_back(offset + 1); indices.push_back(offset + 2);
        indices.push_back(offset + 2); indices.push_back(offset + 3); indices.push_back(offset + 0);
    }

    return BuildMesh(vertices, indices);
}

std::shared_ptr<Mesh> Primitives::CreateQuad() {
    std::vector<Vertex> vertices(4);
    vertices[0] = { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(0, 0) };
    vertices[1] = { glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(1, 0) };
    vertices[2] = { glm::vec3(0.5f,  0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(1, 1) };
    vertices[3] = { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(0, 1) };

    std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };
    return BuildMesh(vertices, indices);
}

std::shared_ptr<Mesh> Primitives::CreatePlane(int subs) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int z = 0; z <= subs; ++z) {
        for (int x = 0; x <= subs; ++x) {
            float u = (float)x / subs;
            float v = (float)z / subs;

            float px = (u - 0.5f) * 10.0f;
            float pz = (v - 0.5f) * 10.0f;

            vertices.push_back({ glm::vec3(px, 0.0f, pz), glm::vec3(0, 1, 0), glm::vec2(u, v) });
        }
    }

    for (int z = 0; z < subs; ++z) {
        for (int x = 0; x < subs; ++x) {
            int topLeft = (z * (subs + 1)) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * (subs + 1)) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    return BuildMesh(vertices, indices);
}

std::shared_ptr<Mesh> Primitives::CreateSphere(float radius, int sectors, int stacks) {
    return CreateCapsule(radius, 0.0f, sectors, stacks);
}

std::shared_ptr<Mesh> Primitives::CreateCapsule(float radius, float cylinderHeight, int sectors, int stacks) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = cylinderHeight * 0.5f;

    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / stacks;
        float phi = v * glm::pi<float>();

        for (int j = 0; j <= sectors; ++j) {
            float u = (float)j / sectors;
            float theta = u * (glm::pi<float>() * 2.0f);

            float x = glm::cos(theta) * glm::sin(phi);
            float y = glm::cos(phi);
            float z = glm::sin(theta) * glm::sin(phi);

            glm::vec3 normal = glm::vec3(x, y, z);
            glm::vec3 position = normal * radius;

            if (y > 0.0f) {
                position.y += halfHeight;
            }
            else if (y < 0.0f) {
                position.y -= halfHeight;
            }

            vertices.push_back({ position, glm::normalize(normal), glm::vec2(u, v) });
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int first = (i * (sectors + 1)) + j;
            int second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    return BuildMesh(vertices, indices);
}

std::shared_ptr<Mesh> Primitives::CreateCylinder(float radius, float height, int sectors) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    float halfHeight = height * 0.5f;

    vertices.push_back({ glm::vec3(0, halfHeight, 0), glm::vec3(0, 1, 0), glm::vec2(0.5f, 0.5f) }); // Top center (Index 0)
    vertices.push_back({ glm::vec3(0, -halfHeight, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f) }); // Bottom center (Index 1)

    int baseIndex = 2;

    for (int i = 0; i <= sectors; ++i) {
        float u = (float)i / sectors;
        float theta = u * glm::pi<float>() * 2.0f;

        float x = glm::cos(theta);
        float z = glm::sin(theta);

        glm::vec3 normal(x, 0, z);

        vertices.push_back({ glm::vec3(x * radius, halfHeight, z * radius), normal, glm::vec2(u, 1.0f) });
        vertices.push_back({ glm::vec3(x * radius, -halfHeight, z * radius), normal, glm::vec2(u, 0.0f) });
    }

    for (int i = 0; i < sectors; ++i) {
        int top = baseIndex + (i * 2);
        int bottom = top + 1;
        int nextTop = top + 2;
        int nextBottom = bottom + 2;

        indices.push_back(top); indices.push_back(bottom); indices.push_back(nextTop);
        indices.push_back(nextTop); indices.push_back(bottom); indices.push_back(nextBottom);

        indices.push_back(0); indices.push_back(nextTop); indices.push_back(top);
        indices.push_back(1); indices.push_back(bottom); indices.push_back(nextBottom);
    }
    return BuildMesh(vertices, indices);
}