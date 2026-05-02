#pragma once

#include "glm/glm/glm.hpp"
#include "Terrain.h"

struct PhysicsActor {
    glm::vec3 position = glm::vec3(10, 20, 10); // Start high
    glm::vec3 velocity = glm::vec3(0);

    float gravity = -25.0f; // Stronger than earth for "game feel"
    float terminalVelocity = -50.0f;
    bool isGrounded = false;

    void Update(float dt, const Terrain& terrain) {
        // 1. Apply Acceleration (Gravity)
        if (!isGrounded) {
            velocity.y += gravity * dt;
        }

        // 2. Terminal Velocity Clamp
        if (velocity.y < terminalVelocity) velocity.y = terminalVelocity;

        // 3. Integrate Position
        position += velocity * dt;

        // 4. Basic Collision (Terrain Floor)
        float floorHeight = terrain.GetHeightAt(position.x, position.z);
        if (position.y <= floorHeight) {
            position.y = floorHeight; // Snap to floor
            velocity.y = 0;           // Stop falling
            isGrounded = true;
        }
        else {
            isGrounded = false;
        }
    }
};