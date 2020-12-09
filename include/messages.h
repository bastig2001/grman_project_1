#pragma once

#include <string>

// The possible Types of Messages a Message-Object can represent.
enum class MessageType {
    NoMessage,
    LogMessage,
    Stop
};

struct Message {
    const MessageType type;

    virtual ~Message() = default;

    template<typename T>
    T* cast() {
        return static_cast<T*>(this);
    }

    operator std::string() const;

  protected:
    Message(MessageType type): type{type} {}
};

struct NoMessage: Message {
    NoMessage(): Message(MessageType::NoMessage) {}
};

struct LogMessage: Message {
    const std::string content;

    LogMessage(
        std::string content
    ): Message(MessageType::LogMessage), 
       content{content} 
    {}
};

struct Stop: Message {
    Stop(): Message(MessageType::Stop) {}
};
