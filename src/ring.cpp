#include "ring.h"
#include "event.h"
#include "message.h"
#include "presenter.h"

#include <set>
#include <random>

using namespace std;

vector<unsigned int> get_unique_ids(size_t number_of_ids);


Ring::Ring(
    size_t number_of_workers, 
    unsigned int worker_sleeptime, 
    Presenter* presenter
): presenter{presenter}
{
    create_workers(number_of_workers, worker_sleeptime);
    set_worker_neighbours();

    presenter->show(CreateEvent::ring_created(number_of_workers));
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
        presenter->show(CreateEvent::worker_created(ids[i], i));
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
        neighbours.insert(neighbours.end(), workers.begin(), it + 1);

        (*it)->set_colleagues(move(neighbours));
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
    presenter->show(CreateEvent::ring_starts());

    worker_threads.clear();
    worker_threads.reserve(workers.size());

    running = true;

    for (unsigned int i{0}; i < workers.size(); i++) {
        worker_threads.push_back(thread{ref(*workers[i])});
        presenter->show(CreateEvent::worker_started(workers[i]->id, i));
    }

    presenter->show(CreateEvent::ring_started());
}

void Ring::start_election() {
    if (worker_threads.size() > 0) {
        workers[0]->assign_message(new StartElection());
    }
}

bool Ring::start_election_at_position(unsigned int worker_position) {
    if (worker_position < workers.size()) {
        workers[worker_position]->assign_message(new StartElection());
        return true;
    }
    else {
        return false;
    }
}

vector<tuple<unsigned int, unsigned int, string>> Ring::get_worker_list() {
    vector<tuple<unsigned int, unsigned int, string>> list{};
    list.reserve(workers.size());

    for (unsigned int i{0}; i < workers.size(); i++) {
        unsigned int id{workers[i]->id};
        unsigned int position{i};
        const string& status{workers[i]->is_running() ? "running" : "stopped"};

        list.push_back(tuple<unsigned int, unsigned int, string>{
            id, position, status
        });
    }

    return list;
}

void Ring::stop() {
    presenter->show(CreateEvent::ring_stops());

    for (unsigned int i{0}; i < worker_threads.size(); i++) {
        if (worker_threads[i].joinable() 
            && 
            workers[i]->is_running()
        ) {
            workers[i]->assign_message(new Stop());
            worker_threads[i].join();
            presenter->show(CreateEvent::worker_Stopped(workers[i]->id, i));
        }
    }

    worker_threads.clear();

    running = false;

    presenter->show(CreateEvent::ring_stopped());
}

Ring::~Ring() {
    if (running) {
        stop();
    }
    
    for (Worker* worker : workers) {
        delete worker;
    }
}


#ifdef UNIT_TEST
#include <catch2/catch.hpp>

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
