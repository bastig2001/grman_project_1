#pragma once

#include "worker.h"

#include <vector>
#include <thread>

// A Ring meant to construct and manage Workers in form of a ring topology.
class Ring {
  private:
    std::vector<Worker*> workers{};
    std::vector<std::thread> worker_threads{};

  public:
    Ring(size_t number_of_workers);
    ~Ring();

    // Starts all Workers in separate threads.
    void start();

    // Starts an election among the Workers
    // according to the Chang and Roberts Algorithm.
    void start_election();

    // Stops all Workers and joins their threads.
    void stop();
};
