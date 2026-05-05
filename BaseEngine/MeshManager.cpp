#include "MeshManager.h"
#include "Loader.h"
#include <iostream>

std::shared_ptr<Mesh> MeshManager::GetMesh(const std::string& filePath) {
    // Check if it's already loaded
    auto it = m_Meshes.find(filePath);
    if (it != m_Meshes.end()) {
        return it->second;
    }

    // Load it 
    Loader loader{};
    loader.LoadModel(filePath);

    auto newMesh = std::make_shared<Mesh>();

    newMesh->SetVertexData(loader.vertices);
    newMesh->SetIndexData(loader.indices);

    // SAVE TO CPU FOR PHYSICS!
    newMesh->vertices = loader.vertices;
    newMesh->indices = loader.indices;

    if (!newMesh->vertices.empty()) {
        newMesh->localAABB = Mesh::CalculateLocalAABB(newMesh->vertices);
    }

    newMesh->Init();

    m_Meshes[filePath] = newMesh;
    return newMesh;
}

void MeshManager::RemoveMesh(const std::string& filePath) {
    auto it = m_Meshes.find(filePath);
    if (it != m_Meshes.end()) {
        m_Meshes.erase(it);
        std::cout << "Mesh removed from manager: " << filePath << std::endl;
    }
}

void MeshManager::Clear() {
    m_Meshes.clear();
}