#include "Object.h"

uint32_t Object::nextID = 0;

// Automatically assign the unique ID when created
Object::Object() : id(++nextID), name("GameObject")
{
}

Object::~Object()
{
    CleanUp();
}

void Object::Init()
{
    // Any base initialization logic goes here
}

void Object::Update(float dt)
{
    // Update all attached components
    for (auto& comp : components) {
        comp->Update(dt);
    }
}

void Object::CleanUp()
{
    components.clear();
}

void Object::SetName(const std::string& newName)
{
    name = newName;
}

void Object::SetRotation(const glm::vec3& newRotation)
{
    localTransform.rotation = newRotation;
    transformDirtyFlag = true;
}

void Object::SetPosition(const glm::vec3& newPosition)
{
    localTransform.position = newPosition;
    transformDirtyFlag = true;
}

void Object::SetScale(const glm::vec3& newScale)
{
    localTransform.scale = newScale;
    transformDirtyFlag = true;
}