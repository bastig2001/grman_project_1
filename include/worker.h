#pragma once

#include "message_buffer.h"
#include "presenters/presenter.h"

#include <chrono>

// The return type for act_upon_message.
// Represents the decision if the loop in operator() should continue, 
//                      or if it should finish.
using ContinueOperation = bool;

// A Worker as a node in the Ring.
class Worker {
 #ifdef UNIT_TEST 
  public: // Needed for the unit tests to be able to examine inner workings
 #else
  private:
 #endif
    unsigned int id;
    Worker* neighbour;
    MessageBuffer message_buffer;
    bool is_leader{false};
    bool participates_in_election{false};
    std::chrono::milliseconds sleeptime;
    Presenter* presenter;
    bool running{false};

    void set_presenter(Presenter* presenter);

    ContinueOperation act_upon_message(Message* message);
    void start_election();
    void participate_in_election(ElectionProposal* proposal);
    void forward_election_proposal(ElectionProposal* proposal);
    void be_elected();
    void propose_oneself();
    void end_election(Elected* elected);

  public:
    Worker(
        unsigned int id, 
        unsigned int sleeptime,
        Presenter* presenter,
        Worker* neighbour = nullptr
    ): id{id}, 
       neighbour{neighbour},
       sleeptime{sleeptime}   
    {
        set_presenter(presenter);
    }

    ~Worker();

    // Assigns a Message to the Worker's Message Buffer for execution,
    void assign_message(Message* message);

    // The execution loop which handles all incoming Messages 
    // and implements the functionalities for the concrete ring node.
    void operator()();

    // Sets the neighbour of the Worker to the given pointer.
    // Throws invalid_argument when the argument is a null pointer.
    void set_neighbour(Worker* neighbour);

    // if the worker is in the method operator()
    bool is_running() const;
};
