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

    std::shared_ptr<Mesh> GetMesh(const std::string& filePath);

    void RemoveMesh(const std::string& filePath);

    void Clear();

private:
    MeshManager() = default;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
};