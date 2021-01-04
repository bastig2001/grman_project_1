#include "ring.h"
#include "presenters/no_presenter.h"

#include <set>
#include <random>

using namespace std;

vector<unsigned int> get_unique_ids(size_t number_of_ids);


Ring::Ring(
    size_t number_of_workers, 
    unsigned int worker_sleeptime, 
    Presenter* presenter
) {
    if (!presenter) {
        presenter = new NoPresenter();
    }
    this->presenter = presenter;

    create_workers(number_of_workers, worker_sleeptime);
    set_worker_neighbours();

    presenter->ring_created(number_of_workers);
}

void Ring::create_workers(
    size_t number_of_workers, 
    unsigned int worker_sleeptime
) {
    workers.reserve(number_of_workers);
    auto ids{get_unique_ids(number_of_workers)};
    
    for (unsigned int i{0}; i < number_of_workers; i++) {
        workers.push_back(
            new Worker(ids[i], i, worker_sleeptime, presenter)
        );
    }
}

void Ring::set_worker_neighbours() {
    for (auto it{workers.begin()}; it < workers.end(); it++) {
        auto first_neighbour{
            it + 1 == workers.end()
            ? workers.begin()
            : it + 1
        };
        vector<Worker*> neighbours{first_neighbour, workers.end()};
        neighbours.insert(neighbours.end(), workers.begin(), it);

        (*it)->set_neighbours(move(neighbours));
    }
}

vector<unsigned int> get_unique_ids(size_t number_of_ids) {
    unsigned int max_id{
        number_of_ids < 100 
            ? 999 
            : (unsigned int)number_of_ids * 10
    };
    set<unsigned int> ids{};
    random_device rd{};
    mt19937 gen{rd()};
    uniform_int_distribution<unsigned int> dis{0, max_id};

    unsigned int i{0};
    while (i < number_of_ids) {
        bool was_inserted{ids.insert(dis(gen)).second};

        if (was_inserted) {
            i++;
        }
    }

    return vector<unsigned int>{ids.begin(), ids.end()};
}

void Ring::start() {
    presenter->ring_starts();

    worker_threads.clear();
    worker_threads.reserve(workers.size());

    running = true;

    for (unsigned int i{0}; i < workers.size(); i++) {
        worker_threads.push_back(thread{ref(*workers[i])});
    }

    presenter->ring_started();
}

void Ring::start_election() {
    if (worker_threads.size() > 0) {
        workers[0]->assign_message_async(new StartElection());
    }
}

void Ring::stop() {
    presenter->ring_stops();

    for (unsigned int i{0}; i < worker_threads.size(); i++) {
        if (worker_threads[i].joinable() 
            && 
            workers[i]->is_running()
        ) {
            workers[i]->assign_message_async(new Stop());
            worker_threads[i].join();
        }
    }

    worker_threads.clear();

    running = false;

    presenter->ring_stopped();
}

Ring::~Ring() {
    if (running) {
        stop();
    }
    
    for (Worker* worker : workers) {
        delete worker;
    }

    // if the presenter is no presenter it needs to be deleted
    NoPresenter* no_presenter{dynamic_cast<NoPresenter*>(presenter)};
    if (no_presenter) {
        delete no_presenter;
    }
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"

TEST_CASE(
    "get_unique_ids returns the request amount of unique and random ids", 
    "[get_unique_ids]"
) {
    size_t size{GENERATE(5, 12, 21, 100)};
    auto ids{get_unique_ids(size)};

    REQUIRE(ids.size() == size);

    SECTION("there are no duplicate ids and they are randomized") {
        set<unsigned int> unique_ids{ids.begin(), ids.end()};

        REQUIRE(unique_ids.size() == size);

        SECTION("the ids are not always the same") {
            vector<unsigned int> other_ids{get_unique_ids(size)};
            unique_ids.insert(other_ids.begin(), other_ids.end());

            CHECK(unique_ids.size() > size);
        }
    }
}

#endif
