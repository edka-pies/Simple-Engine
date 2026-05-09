#pragma once
#include <glm/glm/glm.hpp>
#include <vector>
#include "Object.h"
#include "Terrain.h"

enum class EnemyState {
    Idle,
    Chase
};

class Enemy {
public:
    EnemyState state = EnemyState::Idle;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    float speed = 6.0f;          // Slower than player dash, faster than walking!
    float aggroRadius = 15.0f;   // How close you must be to wake it up
    float gravity = -15.0f;
    bool isGrounded = false;

    Object* visualObject = nullptr;

    // The Update loop needs to know where the player is to chase them!
    void Update(float dt, glm::vec3 playerPos, Object* playerObj, const Terrain& terrain, const std::vector<Object*>& sceneObjects);
};