#pragma once
#include "Component.h"
#include "Object.h"
#include "Scene.h"
#include <glm/glm/glm.hpp>

class PlatformComponent : public Component {
public:
    Scene* scene;
    glm::vec3 startPos;
    glm::vec3 endPos;
    float speed = 3.0f; // Units per second

    float progress = 0.0f;
    int direction = 1; // 1 for forward, -1 for backward

    PlatformComponent(Scene* s) : scene(s) {}

    virtual std::string GetComponentName() const override { return "PlatformComponent"; }

    virtual void Update(float dt) override {
        if (!owner) return;

        // 1. Calculate how long the trip should take
        float distance = glm::distance(startPos, endPos);
        if (distance <= 0.001f) return;

        float timeToComplete = distance / speed;

        // 2. Advance the progress bar (0.0 to 1.0)
        progress += (dt / timeToComplete) * direction;

        if (progress >= 1.0f) {
            progress = 1.0f;
            direction = -1;
        }
        else if (progress <= 0.0f) {
            progress = 0.0f;
            direction = 1;
        }

        // 3. Move the platform!
        glm::vec3 oldPos = owner->GetTransform().position;
        glm::vec3 newPos = glm::mix(startPos, endPos, progress);

        owner->SetPosition(newPos);

        // 4. (Optional) ECS Player Carrying Logic
        // If you want the platform to carry the player, you just check the player's 
        // distance to the platform and add the delta (newPos - oldPos) to the player's position!
    }
};