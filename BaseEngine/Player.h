#pragma once
#include <memory>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Terrain.h"
#include "PhysicsUtils.h"

class Mesh;
class Object;

class Player {
public:
    glm::vec3 position = glm::vec3(30.0f, 10.0f, 10.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    // Physical Constants
    float gravity = -28.0f;
    float acceleration = 50.0f;
    float friction = 10.0f;
    float maxSpeed = 12.0f;
    float jumpForce = 10.0f;

    bool isGrounded = false;

    void Update(float dt, glm::vec3 inputDir, bool wantJump, const Terrain& terrain, const std::vector<Object*>& sceneObjects);
    glm::mat4 GetModelMatrix() const;
    Object* visualObject = nullptr; 

    AABB GetPlayerAABB() const {
        AABB box;
        float radius = 1.0f;
        float height = 2.0f;

        // Center is player position. Box extends out by radius, and up by height.
        box.minBounds = position - glm::vec3(radius, 0.0f, radius);
        box.maxBounds = position + glm::vec3(radius, height, radius);

        return box;
    }
};
