#pragma once
#include "Component.h"
#include "PhysicsUtils.h" // Assuming you have a struct for this!
#include <glm/glm/glm.hpp>
#include <functional>
#include "Object.h"
#include "MeshComponent.h"

class CollisionComponent : public Component {
public:
    AABB localBounds;
    bool isStatic = false; // True for platforms, False for ragdoll parts

    // Optional callback for when this object hits something (like dealing damage!)
    std::function<void(Object*)> onCollisionEnter = nullptr;

    CollisionComponent(AABB bounds, bool staticObj = false)
        : localBounds(bounds), isStatic(staticObj) {
    }

    // Helper to get the world-space bounding box based on the Object's current transform
    AABB GetWorldBounds() {
        AABB targetBox = this->localBounds;

        // --- NEW: THE HITBOX FAILSAFE ---
        // If our loaded box is totally empty (0,0,0), borrow the mesh's box!
        if (targetBox.minBounds == glm::vec3(0.0f) && targetBox.maxBounds == glm::vec3(0.0f)) {
            auto meshComp = owner->GetComponent<MeshComponent>();
            if (meshComp && meshComp->mesh) {
                targetBox = meshComp->mesh->localAABB;
                this->localBounds = targetBox; // Save it so we don't have to search next frame!
            }
        }

        glm::vec3 pos = owner->GetTransform().position;
        glm::vec3 scale = owner->GetTransform().scale;

        AABB worldBox;
        // Multiply by the Object's scale so big cubes have big hitboxes!
        worldBox.minBounds = pos + (targetBox.minBounds * scale);
        worldBox.maxBounds = pos + (targetBox.maxBounds * scale);

        return worldBox;
    }

    virtual void Update(float dt) override {
        // The Collision Component doesn't do the math itself!
        // Your engine's Scene or PhysicsSystem should loop through all CollisionComponents 
        // and push them apart if they intersect.
    }

    virtual std::string GetComponentName() const override { return "CollisionComponent"; }

    virtual void Serialize(json& j) override {
        j["isStatic"] = isStatic;
        j["minX"] = localBounds.minBounds.x;
        j["minY"] = localBounds.minBounds.y;
        j["minZ"] = localBounds.minBounds.z;
        j["maxX"] = localBounds.maxBounds.x;
        j["maxY"] = localBounds.maxBounds.y;
        j["maxZ"] = localBounds.maxBounds.z;
    }

    virtual void Deserialize(const json& j) override {
        isStatic = j.value("isStatic", false);
        localBounds.minBounds = glm::vec3(j.value("minX", -0.5f), j.value("minY", -0.5f), j.value("minZ", -0.5f));
        localBounds.maxBounds = glm::vec3(j.value("maxX", 0.5f), j.value("maxY", 0.5f), j.value("maxZ", 0.5f));
    }
};