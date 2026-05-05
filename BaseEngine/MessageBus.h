#pragma once
#include "Message.h"
#include <memory>
#include <queue>
#include <vector>
#include <unordered_map>
#include <functional>

using MessageCallback = std::function<void(Message*)>;

class MessageBus
{
public:
    // Singleton access (so all systems talk to the same bus)
    static MessageBus& GetInstance()
    {
        static MessageBus instance;
        return instance;
    }

    // Subscribe to a specific message type
    void Subscribe(MessageType type, MessageCallback callback);

    // Push a message into the queue (takes ownership via unique_ptr)
    void EnqueueMessage(std::unique_ptr<Message> message);

    // Process all messages in the queue (Called once per frame)
    void DispatchMessages();

private:
    MessageBus() = default;
    ~MessageBus() = default;

    std::queue<std::unique_ptr<Message>> messageQueue;

    std::unordered_map<MessageType, std::vector<MessageCallback>> subscribers;
};

class ObjectDeletedMessage : public Message
{
public:
    ObjectDeletedMessage(class Object* target)
        : Message(MessageType::ObjectDeleted), object(target) {
    }

    class Object* object;
};