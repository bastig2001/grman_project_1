#pragma once

#include "messages.h"

#include <condition_variable>
#include <mutex>
#include <memory>

// A Message Buffer, which holds at most one element, 
// meant for Communication between Workers.
class MessageBuffer {
  private:
    Message* message;
    bool message_assigned{false};
    std::mutex mtx;
    std::condition_variable message_takable;
    std::condition_variable message_assignable;

  public:
    // Assigns a a given Message to the Buffer.
    // Blocks when the previously assigned Message  
    //   hasn't been taken yet, until it is taken.
    void assign(Message* message);

    // Returns the Message the Buffer holds;
    // Blocks when there is no Message assigned after 
    //   the last one has been taken, until there is.
    Message* take();

    // Returns if the Buffer has no Message
    bool is_empty();
};
