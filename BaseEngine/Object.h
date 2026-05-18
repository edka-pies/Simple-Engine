#pragma once
#include <glm/glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <stdint.h>
#include "Transform.h"
#include "Component.h"

class Object
{
private:
    // Static counter shared across all objects to generate unique IDs
    static uint32_t nextID;

public:
    const uint32_t id; // Permanent, unique ID for this specific object
    std::string name;
    glm::vec3 velocity = glm::vec3(0.0f);

    bool serialize = true;

    Object();
    virtual ~Object();

    void Init();
    void CleanUp();
    virtual void Update(float dt);

    inline std::string GetName() const { return name; }
    void SetName(const std::string& newName);

    // Transform Setters
    void SetRotation(const glm::vec3& newRotation);
    void SetPosition(const glm::vec3& newPosition);
    void SetScale(const glm::vec3& newScale);

    // Transform Getters
    glm::vec3 GetPosition() const { return localTransform.position; }
    glm::vec3 GetRotation() const { return localTransform.rotation; }
    glm::vec3 GetScale() const { return localTransform.scale; }

    const glm::mat4& GetModelMatrix() const {
        if (transformDirtyFlag) {
            cachedModelMatrix = localTransform.mat4();
            transformDirtyFlag = false;
        }
        return cachedModelMatrix;
    }

    glm::mat4 GetTransformMatrix() {
        // 1. Start with a blank identity matrix
        glm::mat4 matrix = glm::mat4(1.0f);

        // 2. Apply Translation (Position)
        matrix = glm::translate(matrix, GetTransform().position);

        // 3. Apply Rotation (Assuming Euler angles in radians)
        matrix = glm::rotate(matrix, GetTransform().rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, GetTransform().rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, GetTransform().rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        // 4. Apply Scale
        matrix = glm::scale(matrix, GetTransform().scale);

        return matrix;
    }

    inline Transform& GetTransform() { return localTransform; }

    // --- COMPONENT SYSTEM ---
    std::vector<std::shared_ptr<Component>> components;

    template<typename T>
    void AddComponent(std::shared_ptr<T> component) {
        component->owner = this;
        components.push_back(component);
        component->Awake();
    }

    template<typename T>
    T* GetComponent() {
        for (auto& comp : components) {
            T* target = dynamic_cast<T*>(comp.get());
            if (target) return target;
        }
        return nullptr;
    }

	std::vector<std::shared_ptr<Component>>& GetComponents() { return components; }

private:
    Transform localTransform;
    mutable glm::mat4 cachedModelMatrix = glm::mat4(1.0f);
    mutable bool transformDirtyFlag = true;
};