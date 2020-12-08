#pragma once

#include "message_buffer.h"

class Worker {
  private:
    int id;
    Worker& neighbour;
    MessageBuffer message_buffer;
    bool is_leader{false};
    bool participates_at_election{false};

  public:
    Worker(int id, Worker& neighbour): id{id}, neighbour{neighbour} {}

    void assign_message(Message message);
    void operator()();
};
