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
	PrimitiveSpawned,
    SceneLoaded,
    CreateLight,
	DeleteLight,
    ChangeMesh,
	LoadLevel,
    SaveLevel,
};

enum class PrimitiveShape {
    Cube,
    Quad,
    Plane,
    Sphere,
    Cylinder,
    Capsule
};

// 2. The Base Message Class
class Message
{
public:
    virtual ~Message() = default;
    MessageType GetType() const { return type; }

protected:
    Message(MessageType t) : type(t) {}
    MessageType type;
};

class ObjectMovedMessage : public Message
{
public:
    ObjectMovedMessage(class Object* target, const glm::vec3& newPos)
        : Message(MessageType::ObjectMoved), object(target), position(newPos) {}

    class Object* object;
    glm::vec3 position;
};

class ObjectSpawnedMessage : public Message
{
public:
    ObjectSpawnedMessage(const std::string& modelFilePath)
        : Message(MessageType::ObjectSpawned), filePath(modelFilePath) {}

    std::string filePath;
};

class CreateLightMessage : public Message {
public:
    CreateLightMessage(LightType t)
        : Message(MessageType::CreateLight), typeToCreate(t) {}

    LightType typeToCreate;
};

class DeleteLightMessage : public Message {
public:
    DeleteLightMessage(Light* target) 
        : Message(MessageType::DeleteLight), lightToDelete(target) {}
    Light* lightToDelete;
};

class PrimitiveSpawnedMessage : public Message {
public:
    PrimitiveSpawnedMessage(PrimitiveShape shape) 
        : Message(MessageType::PrimitiveSpawned), shapeType(shape) {}
    PrimitiveShape shapeType;
};

class ChangeMeshMessage : public Message {
public:
    Object* targetObject;
    std::string newFilePath;

    ChangeMeshMessage(Object* target, const std::string& path)
        : Message(MessageType::ChangeMesh), targetObject(target), newFilePath(path) {}
};

class LoadLevelMessage : public Message {
public:
    std::string levelName;
    LoadLevelMessage(const std::string& name) : Message(MessageType::LoadLevel), levelName(name) {}
};

class SaveLevelMessage : public Message {
public:
    std::string levelName;
    SaveLevelMessage(const std::string& name) : Message(MessageType::SaveLevel), levelName(name) {}
};