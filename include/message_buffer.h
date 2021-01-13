#pragma once

#include "message.h"

#include <condition_variable>
#include <mutex>


// A Message Buffer, which holds at most one element, 
// meant for Communication between Workers
class MessageBuffer {
  private:
    Message* message;
    bool message_assigned{false};
    bool message_is_taken{}; // similar to not message_assigned but meant for assign_and_wait
    std::mutex buffer_mtx, rendezvous_mtx, assign_and_wait_mtx;
    std::condition_variable message_takable;    // if there is a message to be taken
    std::condition_variable message_taken;      // if the message is taken (for assign_and_wait)
    std::condition_variable message_assignable; // if a message can be assigned

  public:
    // assigns a given Message to the Buffer,
    // blocks until the Message has been taken or it times out,
    // waittime is in milliseconds,
    // if it times out, it returns false otherwise true
    bool assign_and_wait(
        Message* message, 
        unsigned int waittime // in ms
    );

    // assigns a given Message to the Buffer,
    // blocks when the previously assigned Message  
    //   hasn't been taken yet, until it is taken
    void assign(Message* message);

    // returns the Message the Buffer holds,
    // blocks when there is no Message assigned after 
    //   the last one has been taken, until there is.
    Message* take();

    // returns whether the Buffer has no Message
    bool is_empty();

    ~MessageBuffer();
};
