#include "MeshManager.h"
#include "Loader.h"
#include <iostream>

std::shared_ptr<Mesh> MeshManager::GetMesh(const std::string& filePath) {
    // 1. Check if it's already loaded
    auto it = m_Meshes.find(filePath);
    if (it != m_Meshes.end()) {
        return it->second;
    }

    // 2. Load it using your Loader class
    Loader loader{};
    loader.LoadModel(filePath);

    // 3. Create the Mesh as a shared_ptr
    auto newMesh = std::make_shared<Mesh>();
    newMesh->SetVertexData(loader.vertices);
    newMesh->SetIndexData(loader.indices);

    // 4. Send the data to the GPU!
    newMesh->Init();

    // 5. Store and return
    m_Meshes[filePath] = newMesh;
    return newMesh;
}

void MeshManager::RemoveMesh(const std::string& filePath) {
    // Erasing it from the map reduces the shared_ptr reference count by 1.
    auto it = m_Meshes.find(filePath);
    if (it != m_Meshes.end()) {
        m_Meshes.erase(it);
        std::cout << "Mesh removed from manager: " << filePath << std::endl;
    }
}

void MeshManager::Clear() {
    m_Meshes.clear();
}