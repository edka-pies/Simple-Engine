#pragma once
#include <memory>
#include <vector>
#include "Mesh.h"
#include "AssetViewer.h"

class Primitives {
public:
    static std::shared_ptr<Mesh> CreateCube();

    static std::shared_ptr<Mesh> CreatePlane(float width, float depth);

};