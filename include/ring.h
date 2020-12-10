#pragma once

#include "worker.h"

#include <vector>
#include <thread>

class Ring {
  private:
    std::vector<Worker*> workers;
    std::vector<std::thread> worker_threads;

  public:
    Ring(size_t number_of_workers);
    void start();
    void start_election();
    void stop();
    ~Ring();
};
