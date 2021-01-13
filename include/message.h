#pragma once

#include <string>


// The possible Types of Messages a Message-Object can represent
enum class MessageType {
    NoMessage,
    LogMessage,
    Stop,
    StartElection,
    ElectionProposal,
    Elected,
    DeadWorker,
    NewWorker
};


// A Message, meant to represent Information for Communication between Workers,
// It is not instantiable and only meant as a common Base Class.
struct Message {
    // the Type of the Message object
    const MessageType type;

    // a more comfortable form of static casting for Message objects,
    // meant to be used to cast to the appropriate Subclass annotated by 'type'
    template<typename T>
    T* cast_to() {
        return static_cast<T*>(this);
    }

    virtual explicit operator std::string() const {
        return "Non-Specified Message";
    }

    virtual ~Message() = default;

  protected:
    Message(MessageType type): type{type} {}
};


// A Message meant to represent nothing
struct NoMessage: Message {
    NoMessage(): Message(MessageType::NoMessage) {}

    explicit operator std::string() const override {
        return "No Message";
    }
};


// A Message which holds content for log/output
struct LogMessage: Message {
    const std::string content;

    LogMessage(
        std::string content
    ): Message(MessageType::LogMessage), 
       content{content} 
    {}

    explicit operator std::string() const override {
        return "Log Message containing '" + content + "'";
    }
};


// The Signal to stop
struct Stop: Message {
    Stop(): Message(MessageType::Stop) {}

    explicit operator std::string() const override {
        return "Stop";
    }
};


// The Signal to start a new election
struct StartElection: Message {
    StartElection(): Message(MessageType::StartElection) {}

    explicit operator std::string() const override {
        return "Start Election";
    }
};


// A proposal for the election containing the id of the proposed leader
struct ElectionProposal: Message {
    const unsigned int id;

    ElectionProposal(
        unsigned int id
    ): Message(MessageType::ElectionProposal), 
       id{id} 
    {}

    explicit operator std::string() const override {
        return "Election Propsal for " + std::to_string(id);
    }
};


// A message containing the id of the newly elected leader
struct Elected: Message {
    const unsigned int id;

    Elected(unsigned int id): Message(MessageType::Elected), id{id} {}

    explicit operator std::string() const override {
        return "Worker " + std::to_string(id) + " has been elected";
    }
};


// A message containing the position of a worker marked dead 
//      who needs to be removed
struct DeadWorker: Message {
    const unsigned int position;

    DeadWorker(
        unsigned int position
    ): Message(MessageType::DeadWorker), 
       position{position} 
    {}

    explicit operator std::string() const override {
        return "Worker on position " + std::to_string(position) 
             + " is marked dead";
    }
}; 


class Worker; // a forward declaration of Worker for NewWorker

// A message containing the position of and pointer to a newly added worker
struct NewWorker: Message {
    const unsigned int position;

    Worker* worker; 

    NewWorker(
        unsigned int position,
        Worker* worker
    ): Message(MessageType::NewWorker), 
       position{position},
       worker{worker}
    {}

    explicit operator std::string() const override {
        return "There is a new worker on position " + std::to_string(position);
    }
};