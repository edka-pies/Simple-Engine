#pragma once
#include <glm/glm/glm.hpp>
#include <string>
#include "Light.h"

// 1. Define all possible events in your engine here
enum class MessageType
{
    ObjectMoved,
    ObjectSpawned,
    ObjectDeleted,
    SceneLoaded,
    CreateLight,
	DeleteLight,
};

// 2. The Base Message Class
class Message
{
public:
    virtual ~Message() = default;
    MessageType GetType() const { return type; }

protected:
    // Only subclasses can set the type
    Message(MessageType t) : type(t) {}
    MessageType type;
};

// 3. Specific Message Sub-classes (The "Payloads")

class ObjectMovedMessage : public Message
{
public:
    // We pass the object pointer and the new transform data
    ObjectMovedMessage(class Object* target, const glm::vec3& newPos)
        : Message(MessageType::ObjectMoved), object(target), position(newPos) {
    }

    class Object* object;
    glm::vec3 position;
};

class ObjectSpawnedMessage : public Message
{
public:
    ObjectSpawnedMessage(const std::string& modelFilePath)
        : Message(MessageType::ObjectSpawned), filePath(modelFilePath) {
    }

    std::string filePath;
};

class CreateLightMessage : public Message {
public:
    CreateLightMessage(LightType t)
        : Message(MessageType::CreateLight), typeToCreate(t) {
    }

    LightType typeToCreate;
};

class DeleteLightMessage : public Message {
public:
    DeleteLightMessage(Light* target) : Message(MessageType::DeleteLight), lightToDelete(target) {}
    Light* lightToDelete;
};