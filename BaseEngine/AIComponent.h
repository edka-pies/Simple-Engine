#pragma once
#include "Component.h"
#include "Object.h"
#include "ActiveRagdollComponent.h"
#include "Scene.h" // Include the scene!

class AIComponent : public Component {
public:
    Scene* myScene = nullptr;
    ActiveRagdollComponent* ragdoll = nullptr;

    // Pass the Scene in, not the player!
    AIComponent(Scene* scene) : myScene(scene) {}

    virtual void Awake() override {
        ragdoll = owner->GetComponent<ActiveRagdollComponent>();
    }

    virtual void Update(float dt) override {
        // If the player doesn't exist yet, just wait.
        if (!ragdoll || !myScene || !myScene->playerObject) return;

        // Dynamically grab the player every frame!
        Object* targetPlayer = myScene->playerObject;

        glm::vec3 myPos = owner->GetTransform().position;
        glm::vec3 targetPos = targetPlayer->GetTransform().position;

        targetPos.y = myPos.y;

        glm::vec3 toPlayer = targetPos - myPos;
        float distance = glm::length(toPlayer);

        if (distance > 1.5f && distance < 15.0f) {
            ragdoll->movementDirection = glm::normalize(toPlayer);
        }
        else {
            ragdoll->movementDirection = glm::vec3(0.0f);
        }
    }

    virtual std::string GetComponentName() const override { return "AIComponent"; }

    virtual void Serialize(json& j) override {
        // We don't save the Scene pointer (it changes every time the game runs)
        // But in the future, you could save things here like:
        // j["detectionRadius"] = 15.0f;
    }

    virtual void Deserialize(const json& j) override {
        // In the future, load stats here!
    }
};