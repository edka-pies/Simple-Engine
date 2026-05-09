#pragma once
#include <memory>
#include <vector>
#include "Mesh.h"
//#include "AssetViewer.h"

class Primitives {
public:
    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreateQuad();
    static std::shared_ptr<Mesh> CreatePlane(int subdivisions = 10);
    static std::shared_ptr<Mesh> CreateSphere(float radius = 0.5f, int sectors = 36, int stacks = 18);
    static std::shared_ptr<Mesh> CreateCylinder(float radius = 0.5f, float height = 1.0f, int sectors = 36);
    static std::shared_ptr<Mesh> CreateCapsule(float radius = 0.5f, float cylinderHeight = 1.0f, int sectors = 36, int stacks = 18);

private:
    static std::shared_ptr<Mesh> BuildMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};