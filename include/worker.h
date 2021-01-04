#pragma once

#include "message_buffer.h"
#include "presenters/presenter.h"

#include <chrono>
#include <vector>

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
    unsigned int position;
    std::chrono::milliseconds sleeptime;
    
    MessageBuffer message_buffer;
    bool is_leader{false};
    bool participates_in_election{false};
    Presenter* presenter;
    bool running{false};

    // Pointers to all other Workers in the Ring, ordered by sending distance
    // The closest Neighbour to which to send Messages is at index 0.
    std::vector<Worker*> neighbours{};

    void set_presenter(Presenter* presenter);

    ContinueOperation act_upon_message(Message* message);
    void start_election();
    void participate_in_election(ElectionProposal* proposal);
    void forward_election_proposal(ElectionProposal* proposal);
    void be_elected();
    void propose_oneself();
    void end_election(Elected* elected);

    void remove_dead_worker(DeadWorker* dead_worker);
    bool position_is_not_neighbour(unsigned int position);
    void add_new_worker(NewWorker* new_worker);
    unsigned int get_neighbours_index_for_position(unsigned int position);

    void send_to_neighbour(Message* message);

  public:
    Worker(
        unsigned int id, 
        unsigned int position,
        unsigned int sleeptime,
        Presenter* presenter
    ): id{id}, 
       position{position},
       sleeptime{sleeptime}   
    {
        set_presenter(presenter);
    }

    ~Worker();

    // Assigns a Message to the Worker's Message Buffer for execution 
    // and blocks until it's taken.
    void assign_message_sync(Message* message);

    // Assigns a Message to the Worker's Message Buffer for execution 
    // but doesn't for it to be taken, only for it to be received by the Buffer.
    void assign_message_async(Message* message);

    // The execution loop which handles all incoming Messages 
    // and implements the functionalities for the concrete ring node.
    void operator()();

    // Sets the neighbours of the Worker.
    // Throws invalid_argument when the vector is empty.
    void set_neighbours(std::vector<Worker*> neighbours);

    // if the worker is in the method operator()
    bool is_running() const;

    // if two workers are equal is determined by their id
    bool operator==(const Worker&);
    bool operator!=(const Worker&);
};
