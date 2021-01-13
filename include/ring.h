#pragma once

#include "worker.h"
#include "presenter.h"

#include <tuple>
#include <vector>
#include <thread>


// A Ring meant to construct and manage Workers in form of a ring topology
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
        unsigned int worker_sleeptime, // in ms
        Presenter* presenter
    );

    ~Ring();

    // starts all Workers in separate threads
    void start();

    // starts the worker on the given position,
    // returns false when there is no worker on the given position
    bool start(unsigned int worker_position);

    // starts an election among the Workers on position 0 in the ring
    //      according to the Chang and Roberts Algorithm
    void start_election();

    // starts an election among the Workers on the given position in the ring
    //      according to the Chang and Roberts Algorithm,
    // returns false when there is no worker on the given position
    bool start_election_at_position(unsigned int worker_position);

    // returns a vector with the id, position and status of each worker
    std::vector<std::tuple<unsigned int, unsigned int, std::string>> 
        get_worker_list();

    // stops all Workers and joins their threads
    void stop();

    // stops the worker on the given position
    // returns false when there is no worker on the given position
    bool stop(unsigned int worker_position);
};
