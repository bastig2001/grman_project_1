#include "ring.h"

using namespace std;

vector<int> get_unique_ids(size_t number_of_ids);


Ring::Ring(size_t number_of_workers) {
    workers.reserve(number_of_workers);
    auto unique_ids = get_unique_ids(number_of_workers);
    
    if (number_of_workers > 0) {
        workers.push_back(new Worker(unique_ids[0]));
        for (unsigned int i{1}; i < number_of_workers; i++) {
            workers.push_back(new Worker(unique_ids[i], workers[i - 1]));
        }
        workers[0]->set_neighbour(workers[number_of_workers - 1]);
    }
}

vector<int> get_unique_ids(size_t number_of_ids) {
    vector<int> ids;
    ids.reserve(number_of_ids);

    for (unsigned int i{1}; i < number_of_ids; i++) {
        ids[i] = i;
    }

    return ids;
}

void Ring::start() {
    worker_threads.clear();
    worker_threads.reserve(workers.size());

    for (unsigned int i{0}; i < workers.size(); i++) {
        worker_threads[i] = thread{ref(*workers[i])};
    }
}

void Ring::stop() {
    for (thread& worker_thread : worker_threads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}

Ring::~Ring() {
    stop();
    for (Worker* worker : workers) {
        delete worker;
    }
}
