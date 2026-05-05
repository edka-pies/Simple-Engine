#include "Primitives.h"

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

    auto mesh = std::make_shared<Mesh>();
    mesh->SetVertexData(vertices); 
    mesh->SetIndexData(indices);   

    mesh->vertices = vertices;     
    mesh->indices = indices;       
    mesh->localAABB = Mesh::CalculateLocalAABB(vertices);
    
    mesh->Init(); 
    return mesh;
}