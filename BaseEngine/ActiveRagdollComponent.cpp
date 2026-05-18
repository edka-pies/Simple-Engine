#include "ActiveRagdollComponent.h"
#include <iostream>
#include <cmath>
#include "Object.h"
#include "GLFW/Include/glfw3.h"
#include "Scene.h"

void ActiveRagdollComponent::BuildRagdoll(std::vector<Object*> parts) {
    bodyParts = parts;

    // We build the joints right here inside the component!
    // [0] Head, [1] Torso, [2] Left Arm, [3] Right Arm, [4] Legs
    // CORRECT ORDER: Torso (Anchor) is objA, Limb is objB
// Head (Up)
    joints.push_back(SpringConstraint{ bodyParts[1], bodyParts[0], glm::vec3(0.0f, 1.2f, 0.0f), 1000.0f, 50.0f });

    // Left Arm (Right side of screen, usually +X)
    joints.push_back(SpringConstraint{ bodyParts[1], bodyParts[2], glm::vec3(1.5f, 0.0f, 0.0f), 1000.0f, 50.0f });

    // Right Arm (Left side of screen, usually -X)
    joints.push_back(SpringConstraint{ bodyParts[1], bodyParts[3], glm::vec3(-1.5f, 0.0f, 0.0f), 1000.0f, 50.0f });

    // Legs (Down! Notice the Negative Y!)
    joints.push_back(SpringConstraint{ bodyParts[1], bodyParts[4], glm::vec3(0.0f, -1.5f, 0.0f), 1000.0f, 50.0f });

    // Muscle Web
    joints.push_back(SpringConstraint{ bodyParts[2], bodyParts[3], glm::vec3(-3.0f, 0.0f, 0.0f), 600.0f, 40.0f }); // Arm-to-Arm
}

void ActiveRagdollComponent::BreakAllJoints() {
    for (auto& joint : joints) {
        joint.isBroken = true;
    }
    std::cout << "All joints manually snapped!\n";
}

void ActiveRagdollComponent::Jump() {
    // 1. Ground check on the LEGS [4], not the Torso!
    if (bodyParts.size() >= 5 && std::abs(bodyParts[4]->velocity.y) < 0.1f) {

        // 2. Apply jump velocity to EVERY part of the body so they move together
        for (Object* part : bodyParts) {
            if (part) part->velocity.y = 12.0f;
        }
    }
}

void ActiveRagdollComponent::Dash(glm::vec3 dir) {
    if (bodyParts.size() >= 5) {

        // Apply dash velocity to EVERY part of the body
        for (Object* part : bodyParts) {
            if (part) part->velocity += dir * 25.0f;
        }
    }
}

void ActiveRagdollComponent::Update(float dt) {
    if (joints.empty() || bodyParts.empty()) return;

    // --- 1. SPRING FORCES (Hooke's Law) ---
    for (auto& joint : joints) {
        if (joint.isBroken) continue;

        glm::vec3 posA = joint.objA->GetTransform().position;
        glm::vec3 posB = joint.objB->GetTransform().position;

        // 1. Where SHOULD the limb be in the world?
        // (If your torso rotates, you would multiply this offset by the Torso's rotation matrix!)
        glm::vec3 targetPos = posA + joint.localOffset;

        // 2. The Error Vector (From CURRENT position to TARGET position)
        glm::vec3 errorVec = targetPos - posB;
        float errorDistance = glm::length(errorVec);

        if (errorDistance < 0.001f) continue;
        glm::vec3 dirToTarget = errorVec / errorDistance; // Normalized direction

        // --- THE HARD CAP (ANTI-STRETCH / THE LEASH) ---
        float maxStretch = 0.15f;
        if (errorDistance > maxStretch) {
            float excess = errorDistance - maxStretch;

            // Push the limb directly back to the edge of the allowed radius
            glm::vec3 correction = dirToTarget * excess;

            joint.objB->SetPosition(posB + correction);

            // Kill the velocity so it doesn't vibrate violently
            joint.objB->velocity *= 0.5f;

            // Cap the error distance so the spring math below doesn't explode
            errorDistance = maxStretch;
        }

        // --- NEW: VECTOR SPRING MATH (FIXED) ---
    // 1. How hard does the spring want to pull?
        float springForceMag = joint.stiffness * errorDistance;

        // 2. What is the relative velocity?
        glm::vec3 relVel = joint.objB->velocity - joint.objA->velocity;

        // 3. How fast is object B moving TOWARDS the target?
        float velAlongSpring = glm::dot(relVel, dirToTarget);

        // 4. Damping force calculates the resistance based on that velocity
        float dampingForceMag = joint.damping * velAlongSpring;

        // 5. Total Force Magnitude: Spring pulls, Damping resists!
        float totalForceMag = springForceMag - dampingForceMag;

        glm::vec3 totalForce = dirToTarget * totalForceMag;

        // Apply force!
        joint.objB->velocity += totalForce * dt;
        joint.objA->velocity -= (totalForce * 0.1f) * dt;
    }

    // --- 2. HOVER CORE & NECK LIFT ---
    if (bodyParts.size() >= 5 && !joints[0].isBroken) {
        Object* torso = bodyParts[1];

        // Movement Logic (Push the Torso!)
        if (glm::length(movementDirection) > 0.1f) {
            glm::vec3 normalizedDir = glm::normalize(movementDirection);
            torso->velocity.x += normalizedDir.x * moveSpeed * dt;
            torso->velocity.z += normalizedDir.z * moveSpeed * dt;

            // Softened the bounce so it doesn't rip the legs off
            float bounce = sin(glfwGetTime() * 15.0f) * 1.0f;
            torso->velocity.y += bounce * dt;
        }

        // --- THE FIX: DYNAMIC HOVER HEIGHT ---
        // TODO: In the future, change `0.5f` to `terrain->GetHeightAt(torso->GetPosition().x, torso->GetPosition().z)`
        float groundBeneathTorso = this->currentGroundHeight;

        // 2. The Hover Core uses that value directly
        float targetHeadHeight = currentGroundHeight + joints[0].localOffset.y + 0.6f;
        float headError = targetHeadHeight - bodyParts[0]->GetTransform().position.y;

        // --- NEW: THE JUMP FIX ---
        // If headError is negative, we are jumping! Don't let the spring pull us down.
        if (headError < 0.0f) {
            headError = 0.0f;
        }
        // Hover exactly 2.5 units above the ground (Leg rest length is 1.5, so this gives them room to dangle)
        float targetHeight = groundBeneathTorso + 2.5f;

        float heightError = targetHeight - torso->GetTransform().position.y;

        // Lowered the correction force from 60.0f to 30.0f so it's less explosive
        torso->velocity.y += (heightError * 30.0f * dt);
        torso->velocity.y *= 0.9f;

        glm::vec3 currentRot = torso->GetRotation();
        currentRot.x *= 0.85f;
        currentRot.z *= 0.85f;
        torso->SetRotation(currentRot);

        Object* head = bodyParts[0];
        if (headError > 0) head->velocity.y += (headError * 40.0f * dt);

        head->velocity.y *= 0.9f;
        head->velocity.x *= 0.95f;
        head->velocity.z *= 0.95f;
    }

    // --- 3. PHYSICS INTEGRATOR ---
    for (Object* part : bodyParts) {
        if (!part) continue;

        part->velocity.y -= 9.81f * dt;
        part->velocity.x *= 0.90f; // Global friction/air resistance
        part->velocity.z *= 0.90f;

        glm::vec3 currentPos = part->GetTransform().position;
        glm::vec3 newPos = currentPos + (part->velocity * dt);

        part->SetPosition(newPos);
    }

    if (bodyParts.size() > 1 && bodyParts[1] != nullptr) {
        owner->SetPosition(bodyParts[1]->GetPosition());
    }
}