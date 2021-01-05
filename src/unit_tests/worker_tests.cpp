#ifdef UNIT_TEST
#include "worker.h"

#include "catch2/catch.hpp"
#include <tuple>
#include <cmath>
#include <thread>

// sleep is needed since there is another thread
#define sleep() this_thread::sleep_for(chrono::milliseconds(20))

using namespace std;


TEST_CASE(
    "Worker interacts with its neighbour and implements the Chang and Roberts algorithm for elections", 
    "[worker][uses_message_buffer][worker_election]"
) {
    tuple<unsigned int, unsigned int> ids{GENERATE(
        tuple<unsigned int, unsigned int>{2, 8}, 
        tuple<unsigned int, unsigned int>{9, 5}
    )};
    unsigned int dummy_id{get<0>(ids)};
    unsigned int worker_id{get<1>(ids)};

    Worker dummy_worker(dummy_id, 0, 0, nullptr);
    Worker worker(worker_id, 0, 0, nullptr);
    worker.set_neighbours({&dummy_worker});

    thread worker_thread{ref(worker)};
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE(worker.is_running());
    
    SECTION("Worker is able to start election") {
        worker.assign_message_sync(new StartElection());
        sleep();

        CHECK(worker.participates_in_election);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::ElectionProposal);
        CHECK(message->cast_to<ElectionProposal>()->id == worker_id);

        delete message;
    }

    SECTION("Worker is able to participate in election") {
        worker.participates_in_election = false;
        worker.is_leader = GENERATE(true, false);

        worker.assign_message_sync(new ElectionProposal(dummy_id));
        sleep();

        CHECK(worker.participates_in_election);
        CHECK_FALSE(worker.is_leader);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::ElectionProposal);
        CHECK(message->cast_to<ElectionProposal>()->id == max(worker_id, dummy_id));

        delete message;
    }

    SECTION("Worker can handle out of order election proposals") {
        worker.participates_in_election = true;

        worker.assign_message_sync(new ElectionProposal(dummy_id));
        sleep();

        CHECK(worker.participates_in_election);
        REQUIRE(dummy_worker.message_buffer.is_empty() == (dummy_id < worker_id));

        if (dummy_id > worker_id) {
            auto message{dummy_worker.message_buffer.take()};
            REQUIRE(message->type == MessageType::ElectionProposal);
            CHECK(message->cast_to<ElectionProposal>()->id == dummy_id);

            delete message;
        }
    }

    SECTION("Worker can be elected") {
        worker.participates_in_election = true;
        worker.is_leader = false;

        worker.assign_message_sync(new ElectionProposal(worker_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK(worker.is_leader);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::Elected);
        CHECK(message->cast_to<Elected>()->id == worker_id);

        delete message;
    }

    SECTION("Worker acts accordingly when someone is elected") {
        worker.participates_in_election = true;
        worker.is_leader = false;

        worker.assign_message_sync(new Elected(dummy_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK_FALSE(worker.is_leader);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::Elected);
        CHECK(message->cast_to<Elected>()->id == dummy_id);

        delete message;
    }

    SECTION("Worker is able to finish election") {
        worker.participates_in_election = false;
        worker.is_leader = true;

        worker.assign_message_sync(new Elected(worker_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK(worker.is_leader);

        CHECK(dummy_worker.message_buffer.is_empty());
    }

    worker.assign_message_sync(new Stop());
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE_FALSE(worker.is_running());

    worker_thread.join();
}

TEST_CASE(
    "Worker interacts with its neighbours and can detect and correct faults in the ring", 
    "[worker][uses_message_buffer][worker_fault_tolerance]"
) {
    unsigned int number_of_neighbours{GENERATE(11u, 15u)};
    unsigned int worker_position{GENERATE(0u, 1u, 4u, 11u)};

    Worker dummy_worker(0, 0, 0, nullptr);
    Worker worker(0, worker_position, 0, nullptr);
    
    vector<Worker*> neighbours{};
    neighbours.assign(number_of_neighbours, &dummy_worker);
    worker.set_neighbours(move(neighbours));

    thread worker_thread{ref(worker)};
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE(worker.is_running());
    
    SECTION("Worker is able to remove dead neighbour") {
        unsigned int dead_worker_position{GENERATE(3u, 9u)};
        worker.assign_message_sync(new DeadWorker(dead_worker_position));
        sleep();

        CHECK(worker.neighbours.size() == number_of_neighbours - 1);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::DeadWorker);
        CHECK(message->cast_to<DeadWorker>()->position == dead_worker_position);

        delete message;
    }

    SECTION("Worker does not react on a dead Worker Message for its neighbour") {
        unsigned int neighbour_position{
            (worker_position + 1) % (number_of_neighbours + 1)
        };
        worker.assign_message_sync(new DeadWorker(neighbour_position));
        sleep();

        CHECK(worker.neighbours.size() == number_of_neighbours);
        CHECK(dummy_worker.message_buffer.is_empty());
    }

    SECTION("Worker is able to add a new worker to its neighbours on correct index") {
        Worker other_worker(1, 0, 0, nullptr);
        unsigned int new_worker_position{GENERATE(3u, 9u)};
        unsigned int expected_new_worker_index{
            worker.get_neighbours_index_for_position(new_worker_position)
        };
        worker.assign_message_sync(
            new NewWorker(new_worker_position, &other_worker)
        );
        sleep();

        CHECK(worker.neighbours.size() == number_of_neighbours + 1);
        CHECK(worker.neighbours[expected_new_worker_index]->id == other_worker.id);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::NewWorker);
        CHECK(message->cast_to<NewWorker>()->position == new_worker_position);

        delete message;
    }

    worker.assign_message_sync(new Stop());
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE_FALSE(worker.is_running());

    worker_thread.join();
}

#endif // UNIT_TEST
