#pragma once
#include <vector>
#include "Object.h"
#include "CollisionComponent.h"
#include "PhysicsUtils.h"

class PhysicsSystem {
public:
    // Call this once per frame from your main engine loop
    void Update(float dt, const std::vector<Object*>& allObjects);

private:
    // Helper function to push objects apart when they crash
    void ResolveAABBCollision(CollisionComponent* dynamicComp, CollisionComponent* staticComp, const AABB& boxA, const AABB& boxB);
};

