#pragma once
#include "Component.h"
#include "ActiveRagdollComponent.h"
#include "CollisionComponent.h"
#include "MeshComponent.h"
#include "Scene.h"
#include <iostream>

class ExplosiveComponent : public Component {
public:
    Scene* scene;
    float explosionRadius = 8.0f;
    float explosionForce = 550.0f; // Tweak this for bigger launches!
    bool hasExploded = false;

    ExplosiveComponent(Scene* s) : scene(s) {}

    virtual std::string GetComponentName() const override { return "ExplosiveComponent"; }

    virtual void Update(float dt) override {
        if (hasExploded || !scene || !scene->playerObject) return;

        glm::vec3 myPos = owner->GetTransform().position;

        // Find the player's torso safely
        auto playerRagdoll = scene->playerObject->GetComponent<ActiveRagdollComponent>();
        if (!playerRagdoll || playerRagdoll->bodyParts.size() < 2) return;

        glm::vec3 playerPos = playerRagdoll->bodyParts[1]->GetTransform().position;

        // Proximity Trigger!
        if (glm::distance(myPos, playerPos) < 2.5f) {
            Detonate();
        }
    }

private:
    void Detonate() {
        scene->timeScale = 0.2f;
        hasExploded = true;
        glm::vec3 explosionPos = owner->GetTransform().position;
        std::cout << "BOOM! Barrel detonated.\n";

        // 1. Hide and disable the barrel so we don't hit an invisible wall later
        owner->SetScale(glm::vec3(0.0f));
        owner->SetPosition(glm::vec3(0.0f, -1000.0f, 0.0f));

        // 2. Loop through EVERY object to find any Ragdolls (Player AND Enemies)
        for (Object* obj : scene->objects) {
            auto ragdoll = obj->GetComponent<ActiveRagdollComponent>();
            if (ragdoll) {
                ApplyBlast(ragdoll, explosionPos);
            }
        }
        float timeScale = 1.0f;
    }

    void ApplyBlast(ActiveRagdollComponent* ragdoll, glm::vec3 explosionCenter) {
        // Use the Torso to check distance
        glm::vec3 torsoPos = ragdoll->bodyParts[1]->GetTransform().position;
        glm::vec3 blastDir = torsoPos - explosionCenter;
        float distance = glm::length(blastDir);

        if (distance < explosionRadius) {
            // Normalize direction to get a pure vector
            blastDir = glm::normalize(blastDir);

            // Force Falloff (Closer = much bigger blast)
            float forceMultiplier = 1.0f - (distance / explosionRadius);
            float appliedForce = explosionForce * forceMultiplier;

            // 1. SNAP ALL THE BONES
            for (auto& joint : ragdoll->joints) {
                joint.isBroken = true;
            }

            // 2. SEND EVERY LIMB FLYING
            for (Object* part : ragdoll->bodyParts) {
                if (part) {
                    // Add a tiny bit of random chaos so the limbs scatter nicely
                    float chaosX = ((rand() % 100) / 100.0f) * 0.5f;
                    float chaosZ = ((rand() % 100) / 100.0f) * 0.5f;

                    part->velocity.x += (blastDir.x + chaosX) * appliedForce;
                    part->velocity.z += (blastDir.z + chaosZ) * appliedForce;

                    // Massive upward kick
                    part->velocity.y += appliedForce * 1.2f;
                }
            }
        }
    }
};