#pragma once
#include "Object.h"
#include "Player.h"
#include <glm/glm/glm.hpp>

class Platform {
public:
    Object* visualObject = nullptr;

    glm::vec3 startPos;
    glm::vec3 endPos;
    float speed = 3.0f; // Units per second

    // Internal trackers
    float progress = 0.0f;
    int direction = 1; // 1 for forward, -1 for backward

    void Update(float dt, Player& player);
};