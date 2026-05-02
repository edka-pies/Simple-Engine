#include "Player.h"
void Player::Update(float dt, glm::vec3 inputDir, bool wantJump, const Terrain& terrain) {
    // 1. Horizontal Movement (Logic)
    if (glm::length(inputDir) > 0.1f) {
        velocity.x += inputDir.x * acceleration * dt;
        velocity.z += inputDir.z * acceleration * dt;
    }
    else {
        // Apply friction
        float drop = glm::length(glm::vec2(velocity.x, velocity.z)) * friction * dt;
        velocity.x = glm::mix(velocity.x, 0.0f, friction * dt);
        velocity.z = glm::mix(velocity.z, 0.0f, friction * dt);
    }

    // Clamp horizontal speed
    float speed = glm::length(glm::vec2(velocity.x, velocity.z));
    if (speed > maxSpeed) {
        velocity.x = (velocity.x / speed) * maxSpeed;
        velocity.z = (velocity.z / speed) * maxSpeed;
    }

    // 2. Gravity & Jumping
    if (isGrounded) {
        velocity.y = 0;
        if (wantJump) {
            velocity.y = jumpForce;
            isGrounded = false;
        }
    }
    else {
        velocity.y += gravity * dt;
    }

    // 3. Integration (Moving the player)
    position += velocity * dt;

    // 4. Custom Collision (Terrain snap)
    float floorY = terrain.GetHeightAt(position.x, position.z);
    if (position.y <= floorY) {
        position.y = floorY;
        velocity.y = 0;
        isGrounded = true;
    }
    else {
        isGrounded = false;
    }
}

glm::mat4 Player::GetModelMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    // You can add tilt/lean rotation logic here later!
    return model;
}