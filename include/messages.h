#pragma once

#include <string>

// The possible Types of Messages a Message-Object can represent.
enum class MessageType {
    NoMessage,
    LogMessage,
    Stop,
    StartElection,
    ElectionProposal,
    Elected
};

// A Message, meant to represent Information for Communication between Workers.
// It is not instantiable and only meant as a common Base Class.
struct Message {
    // The Type of the Message object
    const MessageType type;

    virtual ~Message() = default;

    // An easier form of static casting for Message objects.
    // Meant to be used to cast to the appropriate Subclass annotated by 'type'.
    template<typename T>
    T* cast_to() {
        return static_cast<T*>(this);
    }

    explicit operator std::string() const;

  protected:
    Message(MessageType type): type{type} {}
};

// A Message meant to represent nothing.
struct NoMessage: Message {
    NoMessage(): Message(MessageType::NoMessage) {}
};

// A Message which holds content for log/output.
struct LogMessage: Message {
    const std::string content;

    LogMessage(
        std::string content
    ): Message(MessageType::LogMessage), 
       content{content} 
    {}
};

// The Signal to stop
struct Stop: Message {
    Stop(): Message(MessageType::Stop) {}
};

// The Signal to start a new election
struct StartElection: Message {
    StartElection(): Message(MessageType::StartElection) {}
};

// A proposal for the election containing the id of the proposed leader.
struct ElectionProposal: Message {
    const unsigned int id;

    ElectionProposal(
        unsigned int id
    ): Message(MessageType::ElectionProposal), 
       id{id} 
    {}
};

// A message containing the id of the newly elected leader.
struct Elected: Message {
    const unsigned int id;

    Elected(unsigned int id): Message(MessageType::Elected), id{id} {}
};
