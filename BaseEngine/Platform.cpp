#include "Platform.h"
#include "PhysicsUtils.h"
#include "Mesh.h"

void Platform::Update(float dt, Player& player) {
    if (!visualObject || !visualObject->GetMesh()) return;

    // 1. CALCULATE MOVEMENT DELTA
    glm::vec3 oldPos = visualObject->GetTransform().position;

    // Calculate total distance to normalize speed
    float totalDist = glm::distance(startPos, endPos);
    if (totalDist < 0.01f) return; // Prevent divide by zero

    // Move progress back and forth between 0.0 and 1.0
    progress += (speed * dt * direction) / totalDist;
    if (progress >= 1.0f) {
        progress = 1.0f;
        direction = -1;
    }
    else if (progress <= 0.0f) {
        progress = 0.0f;
        direction = 1;
    }

    // Apply linear interpolation (Lerp)
    glm::vec3 newPos = startPos + (endPos - startPos) * progress;
    glm::vec3 deltaPos = newPos - oldPos; // This is exactly how far we moved this frame

    visualObject->SetPosition(newPos);

    // 2. THE RIDER TRACTOR BEAM
    AABB localBox = visualObject->GetMesh()->GetLocalAABB();
    AABB platformBox = GetWorldAABB(localBox, visualObject->GetModelMatrix());

    // Create a paper-thin box resting exactly on top of the platform
    AABB rideBox = platformBox;
    rideBox.minBounds.y = platformBox.maxBounds.y;         // Start exactly at the top surface
    rideBox.maxBounds.y = platformBox.maxBounds.y + 0.1f;  // Extend up by 0.1 units

    // Create a box for the player's feet
    AABB playerFeet;
    playerFeet.minBounds = player.position + glm::vec3(-0.4f, 0.0f, -0.4f);
    playerFeet.maxBounds = player.position + glm::vec3(0.4f, 0.5f, 0.4f);

    // If the player's feet touch the tractor beam, carry them!
    if (TestAABBAABB(rideBox, playerFeet)) {
        player.position += deltaPos;
    }
}