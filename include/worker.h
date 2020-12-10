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
    bool participates_in_election{false};

    ContinueOperation act_upon_message(Message* message);
    void start_election();
    void participate_in_election(ElectionProposal* proposal);
    void forward_election_proposal(ElectionProposal* proposal);
    void be_elected();
    void discard_election_proposal(ElectionProposal* proposal);
    void propose_oneself();
    void end_election(Elected* elected);

  public:
    Worker(unsigned int id, Worker* neighbour): id{id}, neighbour{neighbour} {}
    Worker(unsigned int id): id{id}, neighbour{nullptr} {}

    void assign_message(Message* message);
    void operator()();
    void set_neighbour(Worker* neighbour);
};
