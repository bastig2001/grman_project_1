#pragma once

#include <string>

struct Message { 
};

struct LogMessage {
    std::string content;

    LogMessage(std::string content): content{content} {}
};
