#pragma once
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Object; // Forward declaration to prevent circular includes

class Component {
public:
    Object* owner = nullptr; // Who does this component belong to?

    virtual ~Component() = default;

    // Called the exact moment the component is added to the object
    virtual void Awake() {}

    // Called every single frame
    virtual void Update(float dt) {}

    // --- NEW: SERIALIZATION INTERFACE ---
    // Every component must return its exact class name as a string
    virtual std::string GetComponentName() const = 0;

    // Every component handles its own specific data
    virtual void Serialize(json& j) {}
    virtual void Deserialize(const json& j) {}
};
