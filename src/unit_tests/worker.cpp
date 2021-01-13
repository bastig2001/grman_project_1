#ifdef UNIT_TEST
#include "worker.h"

#include <catch2/catch.hpp>
#include <tuple>
#include <cmath>
#include <thread>

// sleep is needed since there is another thread
#define sleep() this_thread::sleep_for(chrono::milliseconds(25))

NoPresenter no_presenter;

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

    Worker dummy_worker(dummy_id, 0, 0, &no_presenter);
    Worker worker(worker_id, 0, 0, &no_presenter);
    worker.set_colleagues({&dummy_worker});

    thread worker_thread{ref(worker)};
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE(worker.is_running());
    
    SECTION("Worker is able to start election") {
        worker.assign_message_and_wait(new StartElection());
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

        worker.assign_message_and_wait(new ElectionProposal(dummy_id));
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

        worker.assign_message_and_wait(new ElectionProposal(dummy_id));
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

        worker.assign_message_and_wait(new ElectionProposal(worker_id));
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

        worker.assign_message_and_wait(new Elected(dummy_id));
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

        worker.assign_message_and_wait(new Elected(worker_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK(worker.is_leader);

        CHECK(dummy_worker.message_buffer.is_empty());
    }

    worker.assign_message_and_wait(new Stop());
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE_FALSE(worker.is_running());

    worker_thread.join();
}

TEST_CASE(
    "Worker interacts with its neighbours and can detect and correct faults in the ring", 
    "[worker][uses_message_buffer][worker_fault_tolerance]"
) {
    unsigned int number_of_workers{GENERATE(12u, 15u)};
    unsigned int worker_position{GENERATE(0u, 1u, 4u, 11u)};

    Worker dummy_worker(0, 0, 0, &no_presenter);
    Worker worker(0, worker_position, 0, &no_presenter);
    
    vector<Worker*> neighbours{};
    neighbours.assign(number_of_workers, &dummy_worker);
    worker.set_colleagues(move(neighbours));

    thread worker_thread{ref(worker)};
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE(worker.is_running());
    
    SECTION("Worker is able to remove dead neighbour") {
        unsigned int dead_worker_position{GENERATE(3u, 9u)};
        unsigned int expected_worker_position{worker.position};
        if (expected_worker_position > dead_worker_position) {
            expected_worker_position--;
        }

        worker.assign_message_and_wait(new DeadWorker(dead_worker_position));
        sleep();

        CHECK(worker.colleagues.size() == number_of_workers - 1);
        CHECK(worker.position == expected_worker_position);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::DeadWorker);
        CHECK(message->cast_to<DeadWorker>()->position == dead_worker_position);

        delete message;
    }

    SECTION("Worker does not react on a dead Worker Message for its neighbour") {
        unsigned int neighbour_position{
            (worker_position + 1) % number_of_workers
        };
        unsigned int expected_worker_position{worker.position};

        worker.assign_message_and_wait(new DeadWorker(neighbour_position));
        sleep();

        CHECK(worker.colleagues.size() == number_of_workers);
        CHECK(worker.position == expected_worker_position);
        CHECK(dummy_worker.message_buffer.is_empty());
    }

    SECTION("Worker is able to add a new worker to its neighbours on correct index") {
        Worker other_worker(1, 0, 0, &no_presenter);
        unsigned int new_worker_position{GENERATE(3u, 9u)};
        unsigned int expected_new_worker_index{
            worker.get_neighbours_index_for_position(new_worker_position)
        };
        unsigned int expected_worker_position{worker.position};
        if (expected_worker_position >= new_worker_position) {
            expected_worker_position++;
        }

        worker.assign_message_and_wait(
            new NewWorker(new_worker_position, &other_worker)
        );
        sleep();

        CHECK(worker.colleagues.size() == number_of_workers + 1);
        CHECK(worker.colleagues[expected_new_worker_index]->id == other_worker.id);
        CHECK(worker.position == expected_worker_position);
        REQUIRE_FALSE(dummy_worker.message_buffer.is_empty());

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::NewWorker);
        CHECK(message->cast_to<NewWorker>()->position == new_worker_position);

        delete message;
    }

    worker.assign_message_and_wait(new Stop());
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE_FALSE(worker.is_running());

    worker_thread.join();
}

TEST_CASE(
    "Worker computation functions work as expected",
    "[worker][worker_computations]"
) {
    unsigned int number_of_workers{12};
    unsigned int worker_position{3};

    Worker worker(0, worker_position, 0, &no_presenter);
    
    vector<Worker*> neighbours{};
    neighbours.assign(number_of_workers, &worker);
    worker.set_colleagues(move(neighbours));

    SECTION("Worker is able to calculate the index for a neighbour position") {
        tuple<unsigned int, unsigned int> pos_idx_pair{GENERATE(
            tuple<unsigned int, unsigned int>{5, 1}, 
            tuple<unsigned int, unsigned int>{2, 10},
            tuple<unsigned int, unsigned int>{3, 11},
            tuple<unsigned int, unsigned int>{0, 8},
            tuple<unsigned int, unsigned int>{11, 7}
        )}; 
        unsigned int position{get<0>(pos_idx_pair)};
        unsigned int index{get<1>(pos_idx_pair)};
        
        CHECK(worker.get_neighbours_index_for_position(position) == index);
    }

    SECTION("Worker knows the position of its neighbour") {
        unsigned int number_of_workers{GENERATE(4u, 7u, 12u)};

        vector<Worker*> neighbours{};
        neighbours.assign(number_of_workers, &worker);
        worker.set_colleagues(move(neighbours));

        CHECK(worker.get_direct_neighbour_position() == (worker_position + 1) % number_of_workers);
    }
}

#endif // UNIT_TEST
