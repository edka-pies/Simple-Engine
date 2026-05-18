#include "PhysicsSystem.h"
#include <algorithm>
#include <iostream>

void PhysicsSystem::Update(float dt, const std::vector<Object*>& allObjects) {
    // 1. GATHER PHASE: Find everything that actually has a collider
    std::vector<CollisionComponent*> colliders;
    for (Object* obj : allObjects) {
        if (!obj) continue;
        auto comp = obj->GetComponent<CollisionComponent>();
        if (comp) colliders.push_back(comp);
    }

    if (colliders.size() < 2) return; // Nothing to collide!

    // 2. DETECTION PHASE: Test every collider against every other collider
    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            CollisionComponent* a = colliders[i];
            CollisionComponent* b = colliders[j];

            // Optimization: If both are static (like two floor tiles), don't bother testing them!
            if (a->isStatic == b->isStatic) continue;

            if (a->owner->GetName() == "PlayerBrain" || b->owner->GetName() == "PlayerBrain") {
                std::cout << "Player hit something!\n";
            }

            AABB boxA = a->GetWorldBounds();
            AABB boxB = b->GetWorldBounds();

            if (TestAABBAABB(boxA, boxB)) {
                // 3. RESOLUTION PHASE
                // We always want 'a' to be the moving object for the math below.
                // If 'a' is static and 'b' is dynamic, swap them!
                if (a->isStatic) {
                    ResolveAABBCollision(b, a, boxB, boxA);
                }
                else {
                    ResolveAABBCollision(a, b, boxA, boxB);
                }
            }
        }
    }
}

void PhysicsSystem::ResolveAABBCollision(CollisionComponent* dynComp, CollisionComponent* statComp, const AABB& boxDyn, const AABB& boxStat) {
    Object* dynObj = dynComp->owner;

    // Calculate how far the boxes overlap on each axis
    float overlapX = std::min(boxDyn.maxBounds.x, boxStat.maxBounds.x) - std::max(boxDyn.minBounds.x, boxStat.minBounds.x);
    float overlapY = std::min(boxDyn.maxBounds.y, boxStat.maxBounds.y) - std::max(boxDyn.minBounds.y, boxStat.minBounds.y);
    float overlapZ = std::min(boxDyn.maxBounds.z, boxStat.maxBounds.z) - std::max(boxDyn.minBounds.z, boxStat.minBounds.z);

    glm::vec3 pos = dynObj->GetPosition();

    // Find the axis with the SMALLEST overlap. That is the direction we pushed in from!
    if (overlapX < overlapY && overlapX < overlapZ) {
        // Push out on X
        if (boxDyn.minBounds.x < boxStat.minBounds.x) pos.x -= overlapX; // Pushed left
        else pos.x += overlapX; // Pushed right
        dynObj->velocity.x = 0.0f; // Kill X momentum
    }
    else if (overlapY < overlapX && overlapY < overlapZ) {
        // Push out on Y (This is usually hitting the floor or ceiling)
        if (boxDyn.minBounds.y < boxStat.minBounds.y) {
            pos.y -= overlapY; // Hit ceiling
            if (dynObj->velocity.y > 0) dynObj->velocity.y = 0;
        }
        else {
            pos.y += overlapY; // Hit floor!
            if (dynObj->velocity.y < 0) dynObj->velocity.y = 0;

            // Apply ground friction since we are touching the floor
            dynObj->velocity.x *= 0.95f;
            dynObj->velocity.z *= 0.95f;
        }
    }
    else {
        // Push out on Z
        if (boxDyn.minBounds.z < boxStat.minBounds.z) pos.z -= overlapZ; // Pushed forward
        else pos.z += overlapZ; // Pushed backward
        dynObj->velocity.z = 0.0f; // Kill Z momentum
    }

    // Apply the corrected position immediately
    dynObj->SetPosition(pos);
}