#pragma once

#include "messages.h"

#include <condition_variable>
#include <mutex>

class MessageBuffer {
private:
    Message message;
    bool message_set{false};
    std::mutex mtx;
    std::condition_variable message_takable;
    std::condition_variable message_assignable;

public:
    void push(Message message);
    Message pop();
};
