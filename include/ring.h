#pragma once

#include "worker.h"
#include "presenter.h"

#include <vector>
#include <thread>


// A Ring meant to construct and manage Workers in form of a ring topology.
class Ring {
  private:
    std::vector<Worker*> workers{};
    std::vector<std::thread> worker_threads{};
    Presenter* presenter;
    bool running{false};

    void create_workers(
        size_t number_of_workers, 
        unsigned int worker_sleeptime
    );
    void set_worker_neighbours();

  public:
    Ring(
        size_t number_of_workers, 
        unsigned int worker_sleeptime, 
        Presenter* presenter
    );

    ~Ring();

    // Starts all Workers in separate threads.
    void start();

    // Starts an election among the Workers
    // according to the Chang and Roberts Algorithm.
    void start_election();
    
    // returns true if the election was started successfully
    bool start_election_at_position(unsigned int worker_position);

    // Stops all Workers and joins their threads.
    void stop();
};
