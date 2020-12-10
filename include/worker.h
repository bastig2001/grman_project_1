#pragma once

#include "message_buffer.h"

using ContinueOperation = bool;

class Worker {
  private:
    unsigned int id;
    Worker* neighbour;
    Worker* current_neighbour{nullptr};
    MessageBuffer message_buffer;
    bool is_leader{false};
    bool participates_at_election{false};

    ContinueOperation act_upon_message(Message* message);

  public:
    Worker(unsigned int id, Worker* neighbour): id{id}, neighbour{neighbour} {}
    Worker(unsigned int id): id{id}, neighbour{nullptr} {}

    void assign_message(Message* message);
    void operator()();
    void set_neighbour(Worker* neighbour);
};
