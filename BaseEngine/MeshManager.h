#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Mesh.h"

class MeshManager {
public:
    static MeshManager& GetInstance() {
        static MeshManager instance;
        return instance;
    }

    // Loads a mesh or returns an existing one
    std::shared_ptr<Mesh> GetMesh(const std::string& filePath);

    // Deletes a mesh from the manager at runtime
    void RemoveMesh(const std::string& filePath);

    // Clears everything (useful for level transitions or shutdown)
    void Clear();

private:
    MeshManager() = default;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
};