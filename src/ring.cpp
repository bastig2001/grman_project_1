#include "ring.h"

using namespace std;

vector<unsigned int> get_unique_ids(size_t number_of_ids);


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

vector<unsigned int> get_unique_ids(size_t number_of_ids) {
    vector<unsigned int> ids;
    ids.reserve(number_of_ids);

    for (unsigned int i{0}; i < number_of_ids; i++) {
        ids[i] = i;
    }

    return ids;
}

void Ring::start() {
    worker_threads.clear();
    worker_threads.reserve(workers.size());

    for (unsigned int i{0}; i < workers.size(); i++) {
        worker_threads.push_back(thread{ref(*workers[i])});
    }
}

void Ring::stop() {
    for (unsigned int i{0}; i < worker_threads.size(); i++) {
        if (worker_threads[i].joinable()) {
            workers[i]->assign_message(new Stop());
            worker_threads[i].join();
        }
    }
}

Ring::~Ring() {
    stop();
    for (Worker* worker : workers) {
        delete worker;
    }
}
