#pragma once
#include <vector>

class Object;

struct SpringConstraint {
    Object* objA;
    Object* objB;

    float restLength;    // The distance they "want" to be from each other
    float stiffness;     // How hard the spring pulls (k)
    float damping;       // Prevents infinite bouncing

    bool isBroken = false; // The trigger for your ragdoll effect!
};

class RagdollEntity
{
public:
    std::vector<Object*> bodyParts; // Head, Torso, Arms, Legs
    std::vector<SpringConstraint> joints; // The springs connecting them

    void UpdatePhysics(float dt);
    void BreakAllJoints(); // Call this on your input!
	void ApplyUprightForce(Object* torso, float strength); // Call this in Update to keep it from looking too floppy
    void UpdateAnimation(float time, float moveInput);
};

