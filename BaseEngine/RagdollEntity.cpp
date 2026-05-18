#include "RagdollEntity.h"
#include "glm/glm/glm.hpp"
#include <iostream>
#include "Object.h"

void RagdollEntity::UpdatePhysics(float dt) {
    for (auto& joint : joints) {
        if (joint.isBroken) continue;

        glm::vec3 posA = joint.objA->GetTransform().position;
        glm::vec3 posB = joint.objB->GetTransform().position;

        glm::vec3 delta = posB - posA;
        float currentDist = glm::length(delta);

        if (currentDist < 0.001f) continue;
        glm::vec3 dir = delta / currentDist;

        float displacement = currentDist - joint.restLength;

        // FIX: The breaking logic must go exactly here, INSIDE the loop, AFTER displacement is calculated!
        float breakThreshold = 5.0f;
        if (std::abs(displacement) > breakThreshold) {
            joint.isBroken = true;
            std::cout << "Joint Snapped!\n";
            continue;
        }

        // Apply forces
        float springForceMag = joint.stiffness * displacement;
        glm::vec3 relVel = joint.objB->velocity - joint.objA->velocity;
        float dampingForceMag = joint.damping * glm::dot(relVel, dir);

        glm::vec3 totalForce = dir * (springForceMag + dampingForceMag);

        joint.objA->velocity += totalForce * dt;
        joint.objB->velocity -= totalForce * dt;
    }

    if (bodyParts.size() >= 5 && !joints[0].isBroken) {
        Object* torso = bodyParts[1];

        // 1. The Torso "wants" to hover exactly at Y = 6.5 (1.5 units above ground)
        float targetHeight = 6.5f;
        float heightError = targetHeight - torso->GetTransform().position.y;

        // 2. Apply a massive stabilizing lift
        torso->velocity.y += (heightError * 60.0f * dt);

        // 3. CRITICAL: Air friction for the Torso so it doesn't bounce to the moon
        torso->velocity.y *= 0.9f;

        // 4. Force Upright: Flatten the rotation so it doesn't faceplant
        glm::vec3 currentRot = torso->GetRotation();
        currentRot.x *= 0.85f; // Kill forward/backward tilt
        currentRot.z *= 0.85f; // Kill side-to-side tilt
        torso->SetRotation(currentRot);

        Object* head = bodyParts[0];

        // The head wants to be exactly 'restLength' units ABOVE the torso
        float targetHeadHeight = torso->GetTransform().position.y + joints[0].restLength + 0.2f;
        float headError = targetHeadHeight - head->GetTransform().position.y;

        if (headError > 0) {
            head->velocity.y += (headError * 40.0f * dt);
        }

        // Add drag to the head so it doesn't vibrate violently
        head->velocity.y *= 0.9f;
        head->velocity.x *= 0.95f;
        head->velocity.z *= 0.95f;
    }

    for (Object* part : bodyParts) {
        if (!part) continue;

        // 1. Apply Gravity
        part->velocity.y -= 9.81f * dt;

        part->velocity.x *= 0.98f;
        part->velocity.z *= 0.98f;

        // 2. Calculate New Position (Encapsulation Safe)
        glm::vec3 currentPos = part->GetTransform().position;
        glm::vec3 newPos = currentPos + (part->velocity * dt);

        // 3. Terrain Collision Check
        float groundHeight = 0.5f; // 5.0 terrain + 0.5 cube radius

        if (newPos.y < groundHeight) {
            newPos.y = groundHeight;        // Snap to floor
            part->velocity.y = 0.0f;        // Stop falling

            // Floor Friction
            part->velocity.x *= 0.9f;
            part->velocity.z *= 0.9f;
        }

        // 4. Actually move the object!
        part->SetPosition(newPos);
    }
}

void RagdollEntity::BreakAllJoints()
{
    // Loop through every single joint in the entity...
    for (auto& joint : joints) {
        // ...and forcefully snap them all!
        joint.isBroken = true;
    }

    std::cout << "All joints manually snapped!\n";
}

void RagdollEntity::ApplyUprightForce(Object* torso, float strength) {
    if (!torso) return;

    // Just grab the current rotation and forcefully flatten the X (Pitch) and Z (Roll)
    // This keeps the Y (Yaw) intact so the ragdoll can still turn around, but forces it to stand up!
    glm::vec3 currentRot = torso->GetRotation();
    currentRot.x *= 0.9f;
    currentRot.z *= 0.9f;
    torso->SetRotation(currentRot);
}

void RagdollEntity::UpdateAnimation(float time, float moveInput) {
    if (moveInput == 0.0f) return;

    // A simple sine wave to create a "stepping" motion
    float stepFrequency = 10.0f;
    float stepHeight = 0.5f;
    float legSwing = sin(time * stepFrequency) * 1.5f;

    // Find our Left and Right foot objects
    Object* leftFoot = bodyParts[2];
    Object* rightFoot = bodyParts[3];

    // Apply "kick" forces to move them forward and back
    leftFoot->velocity.z += legSwing * moveInput;
    rightFoot->velocity.z -= legSwing * moveInput;

    // Add a little hop to each step
    if (legSwing > 0) leftFoot->velocity.y += stepHeight;
    else rightFoot->velocity.y += stepHeight;
}