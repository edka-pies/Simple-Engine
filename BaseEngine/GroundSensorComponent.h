#pragma once
#include "Component.h"
#include "ActiveRagdollComponent.h"
#include "Scene.h"

class GroundSensorComponent : public Component {
public:
    Scene* scene;
    ActiveRagdollComponent* ragdoll;

    GroundSensorComponent(Scene* s) : scene(s) {}

    virtual std::string GetComponentName() const override { return "GroundSensorComponent"; }

    virtual void Awake() override {
        ragdoll = owner->GetComponent<ActiveRagdollComponent>();
    }

    virtual void Update(float dt) override {
        if (!ragdoll || !scene) return;

        // Get the Torso's position
        glm::vec3 pos = ragdoll->bodyParts[1]->GetTransform().position;

        // Default to way below the map
        float highestGroundY = -9999.0f;

        // 1. Check the Perlin Terrain
        if (scene->activeTerrain) {
            highestGroundY = scene->activeTerrain->GetTriangleHeightAt(pos.x, pos.z);
        }

        // 2. Check the Level Props (Cubes, Platforms, etc.)
        for (Object* obj : scene->objects) {
            // Don't check against ourselves!
            if (obj->GetName() == "PlayerBrain" || obj->GetName().find("Player_") != std::string::npos) continue;

            auto col = obj->GetComponent<CollisionComponent>();

            if (col && col->isStatic) {
                AABB box = col->GetWorldBounds();

                // Are we standing directly over this box? (Check X and Z)
                bool overX = (pos.x >= box.minBounds.x && pos.x <= box.maxBounds.x);
                bool overZ = (pos.z >= box.minBounds.z && pos.z <= box.maxBounds.z);

                if (overX && overZ) {
                    // Is the top of this box higher than the terrain?
                    // (And make sure we are actually above it, not trapped underneath it)
                    if (box.maxBounds.y > highestGroundY && pos.y >= box.maxBounds.y - 1.0f) {
                        highestGroundY = box.maxBounds.y;
                    }
                }
            }
        }

        // 3. Inject the TRUE ground height into the Ragdoll's Hover Core
        ragdoll->currentGroundHeight = highestGroundY;
    }
};