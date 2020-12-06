#pragma once

#include <string>

enum class MessageType {
    NoMessage,
    LogMessage
};

class Message {
private:
    MessageType type;
    std::string content;

public:
    Message(
        MessageType type = MessageType::NoMessage, 
        std::string content = ""
    ): type{type}, 
       content{content} 
    {}

    MessageType get_type() {
        return type;
    }

    std::string get_content() {
        return content;
    }
};
