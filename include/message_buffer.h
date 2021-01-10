#pragma once

#include "messages.h"

#include <condition_variable>
#include <mutex>

// A Message Buffer, which holds at most one element, 
// meant for Communication between Workers.
class MessageBuffer {
  private:
    Message* message;
    bool message_assigned{false};
    bool message_is_taken{}; // similar to not message_assigned but meant for assign_sync
    std::mutex buffer_mtx, rendezvous_mtx, assign_and_wait_mtx;
    std::condition_variable message_takable;    // if there is a message to be taken
    std::condition_variable message_taken;      // if the message is taken (for assign_and_wait)
    std::condition_variable message_assignable; // if a message can be assigned

  public:
    // Assigns a a given Message to the Buffer.
    // Blocks until the Message has been taken or it times out.
    // Waittime is in milliseconds.
    // If it times out, it returns false otherwise true
    bool assign_and_wait(Message* message, unsigned int waittime);

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
