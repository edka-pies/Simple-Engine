#pragma once
#include "Component.h"
#include <vector>
#include <glm/glm/glm.hpp>
#include "Scene.h"

class Object;

// --- THE MISSING STRUCT ---
struct SpringConstraint {
    Object* objA;
    Object* objB;
    glm::vec3 localOffset;
    float stiffness;
    float damping;
    bool isBroken = false;
};

class ActiveRagdollComponent : public Component {
public:
    float currentGroundHeight = 0.0f;

    std::vector<Object*> bodyParts;
    std::vector<SpringConstraint> joints;

    // Movement variables for the new system!
    glm::vec3 movementDirection = glm::vec3(0.0f);
    float moveSpeed = 150.0f;

    void BuildRagdoll(std::vector<Object*> parts);
    void BreakAllJoints();

    void Jump();
    void Dash(glm::vec3 dir);

    // The Component System calls this automatically every frame
    virtual void Update(float dt) override;

    virtual std::string GetComponentName() const override { return "ActiveRagdollComponent"; }

    virtual void Serialize(json& j) override {
        // We don't save the physical joints (they reset on load), but we CAN save config!
        j["moveSpeed"] = moveSpeed;
    }

    virtual void Deserialize(const json& j) override {
        moveSpeed = j.value("moveSpeed", 5.0f);
    }
};