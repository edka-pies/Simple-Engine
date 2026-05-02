#pragma once
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include "Terrain.h"

class Player {
public:
    glm::vec3 position = glm::vec3(10.0f, 50.0f, 10.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    // Physical Constants
    float gravity = -28.0f;
    float acceleration = 50.0f;
    float friction = 10.0f;
    float maxSpeed = 12.0f;
    float jumpForce = 10.0f;

    bool isGrounded = false;

    void Update(float dt, glm::vec3 inputDir, bool wantJump, const Terrain& terrain);
    glm::mat4 GetModelMatrix() const;
};