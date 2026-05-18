#pragma once
#include "Component.h"
#include "Object.h"
#include "ActiveRagdollComponent.h"
#include "Camera.h"
#include <GLFW/Include/glfw3.h> 

class PlayerInputComponent : public Component {
public:
    GLFWwindow* window;
    Camera* camera; // Pass the camera so we know which way is forward
    ActiveRagdollComponent* ragdoll = nullptr;

    PlayerInputComponent(GLFWwindow* win, Camera* cam) : window(win), camera(cam) {}

    virtual void Awake() override {
        ragdoll = owner->GetComponent<ActiveRagdollComponent>();
    }

    virtual void Update(float dt) override {
        if (!ragdoll || !camera) return;

        glm::vec3 moveDir(0.0f);

        // Get the camera's forward and right vectors, flattened to the ground (Y = 0)
        glm::vec3 forward = camera->GetFront();
        forward.y = 0.0f;
        forward = glm::normalize(forward);

        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;

        if (glm::length(moveDir) > 0.01f) {
            moveDir = glm::normalize(moveDir);

            // --- NEW: WHOLE BODY PUSH ---
            // Instead of just passing the direction, we directly apply velocity to EVERY body part!
            for (Object* part : ragdoll->bodyParts) {
                if (part) {
                    // Adjust this 150.0f speed based on how heavy your physics objects are!
                    part->velocity.x += moveDir.x * 150.0f * dt;
                    part->velocity.z += moveDir.z * 150.0f * dt;
                }
            }
        }

        ragdoll->movementDirection = moveDir;

        static bool spaceWasPressed = false;
        bool spaceIsPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        bool wantJump = (spaceIsPressed && !spaceWasPressed);
        spaceWasPressed = spaceIsPressed;

        if (wantJump) ragdoll->Jump(); // ADD THIS LINE!

        // --- DASH DEBOUNCER & COOLDOWN ---
        static bool shiftWasPressed = false;
        static float dashCooldown = 0.0f; // Track the cooldown

        // Tick the cooldown timer down using delta time
        if (dashCooldown > 0.0f) dashCooldown -= dt;

        bool shiftIsPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        bool wantDash = (shiftIsPressed && !shiftWasPressed);
        shiftWasPressed = shiftIsPressed;

        // Only dash if we are pressing a direction AND the cooldown is finished
        if (wantDash && glm::length(moveDir) > 0.1f && dashCooldown <= 0.0f) {
            ragdoll->Dash(moveDir);
            dashCooldown = 1.0f; // Lock the dash for 1 full second
        }
    }
    // Add these inside the public section of PlayerInputComponent
    virtual std::string GetComponentName() const override { return "PlayerInputComponent"; }

    virtual void Serialize(json& j) override {
        // Input has no saveable data (we can't save a pointer to your keyboard!)
    }

    virtual void Deserialize(const json& j) override {
        // Nothing to load
    }
};