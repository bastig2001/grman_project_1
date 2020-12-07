#pragma once

#include <string>

// The possible Types of Messages a Message-Object can represent.
enum class MessageType {
    NoMessage,
    LogMessage
};

// A Message, meant to represent Information for Communication between Workers.
class Message {
  private:
    MessageType _type;
    std::string _content;

  public:
    Message(
        MessageType type = MessageType::NoMessage, 
        std::string content = ""
    ): _type{type}, 
       _content{content} 
    {}

    // The Type which is represented.
    MessageType type() {
        return _type;
    }

    // The optional content of the Message.
    // Tts use is dependent on the Type.
    std::string content() {
        return _content;
    }
};
