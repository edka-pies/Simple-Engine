#include "MessageBus.h"

void MessageBus::Subscribe(MessageType type, MessageCallback callback)
{
    // Add the callback to the list for this specific message type
    subscribers[type].push_back(callback);
}

void MessageBus::EnqueueMessage(std::unique_ptr<Message> message)
{
    // Push it to the back of the line
    messageQueue.push(std::move(message));
}

void MessageBus::DispatchMessages()
{
    // Process every message currently in the queue
    while (!messageQueue.empty())
    {
        // Get the oldest message
        std::unique_ptr<Message> msg = std::move(messageQueue.front());
        messageQueue.pop();

        // Find if anyone is listening to this specific type of message
        auto it = subscribers.find(msg->GetType());
        if (it != subscribers.end())
        {
            // Call every function that subscribed to this event
            for (const auto& callback : it->second)
            {
                callback(msg.get()); // Pass the raw pointer to the listener
            }
        }
    }
}