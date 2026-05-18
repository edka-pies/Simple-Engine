#pragma once
#include "Component.h"
#include <memory>
#include <string>
#include "Primitives.h"
#include "Mesh.h"
#include "MeshManager.h"

class MeshComponent : public Component {
public:
    std::shared_ptr<Mesh> mesh;
    std::string meshPath; // Useful for saving the level later

    // Constructor to easily assign the mesh when adding the component
    MeshComponent(std::shared_ptr<Mesh> m, const std::string& path = "")
        : mesh(m), meshPath(path) {
    }

    virtual void Awake() override {
        // Optional: Any setup when the mesh is attached
    }

    virtual void Update(float dt) override {
        // Meshes usually don't need to update themselves on the CPU side, 
        // the Renderer handles them!
    }

    virtual std::string GetComponentName() const override { return "MeshComponent"; }

    virtual void Serialize(json& j) override {
        j["meshPath"] = meshPath;
    }

    virtual void Deserialize(const json& j) override {
        meshPath = j.value("meshPath", "");

        if (meshPath == "Primitive_Cube") mesh = Primitives::CreateCube();
        else if (meshPath == "Primitive_Sphere") mesh = Primitives::CreateSphere();
        else if (meshPath == "Primitive_Plane") mesh = Primitives::CreatePlane();
        else if (meshPath == "Primitive_Quad") mesh = Primitives::CreateQuad();
        else if (meshPath == "Primitive_Cylinder") mesh = Primitives::CreateCylinder();
        else if (meshPath == "Primitive_Capsule") mesh = Primitives::CreateCapsule(0.5f, 1.0f);
        else if (!meshPath.empty()) {
            mesh = MeshManager::GetInstance().GetMesh(meshPath);
        }
    }
};