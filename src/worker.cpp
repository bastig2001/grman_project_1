#include "worker.h"
#include "presenters/no_presenter.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace std;


void Worker::assign_message(Message* message) {
    message_buffer.assign_async(message);
}

void Worker::set_neighbour(Worker* neighbour) {
    if (neighbour) {
        this->neighbour = neighbour;
    }
    else {
        throw invalid_argument("Neighbour must point to a valid Worker object.");
    }
}

void Worker::set_presenter(Presenter* presenter) {
    // sets the presenter of the worker to the given presenter 
    // when it's not a null pointer,
    // otherwise it's set to no presenter
    this->presenter = 
        presenter
        ? presenter
        : new NoPresenter();
}

bool Worker::is_running() const {
    return running;
}

Worker::~Worker() {
    // if the presenter of the worker is no presenter it needs to be deleted
    NoPresenter* no_presenter{dynamic_cast<NoPresenter*>(presenter)};
    if (no_presenter) {
        delete no_presenter;
    }
}

void Worker::operator()() {
    if (neighbour) {
        running = true;

        bool continue_operation{true};
        while (continue_operation) {
            this_thread::sleep_for(sleeptime);
            continue_operation = act_upon_message(message_buffer.take());
        }

        running = false;
    }
}

ContinueOperation Worker::act_upon_message(Message* message) {
    ContinueOperation continue_operation{true};

    presenter->worker_got_message(id, message);

    switch (message->type) {
        case MessageType::LogMessage:
            presenter->worker_says(id, message->cast_to<LogMessage>()->content);
            break;
        case MessageType::StartElection:
            start_election();
            break;
        case MessageType::ElectionProposal:
            participate_in_election(message->cast_to<ElectionProposal>());
            break;
        case MessageType::Elected:
            end_election(message->cast_to<Elected>());
            break;
        case MessageType::Stop:
            continue_operation = false;
            break;
        case MessageType::NoMessage:
            break;
    }

    delete message;

    return continue_operation;
}

void Worker::start_election() {
    presenter->worker_starts_election(id);

    presenter->worker_participates_in_election(id);
    participates_in_election = true;

    propose_oneself();
}

void Worker::participate_in_election(ElectionProposal* proposal) {
    if (is_leader) {
        presenter->worker_resigns_as_leader(id);
        is_leader = false;
    }

    bool already_participated_in_election{participates_in_election};

    if (!participates_in_election) {
        participates_in_election = true;
        presenter->worker_participates_in_election(id);
    }

    if (proposal->id > id) {
        forward_election_proposal(proposal);
    }
    else if (proposal->id == id) {
        be_elected();
    }
    else {
        if (already_participated_in_election) {
            presenter->worker_discards_election_proposal(id, proposal->id);
        }
        else {
            propose_oneself();
        }
    }
}

void Worker::forward_election_proposal(ElectionProposal* proposal) {
    presenter->worker_forwards_election_proposal(id, proposal->id);
    neighbour->assign_message(new ElectionProposal(proposal->id));
}

void Worker::be_elected() {
    presenter->worker_is_elected(id);
    is_leader = true;

    presenter->worker_stops_election_participation(id);
    participates_in_election = false;

    neighbour->assign_message(new Elected(id));
}

void Worker::propose_oneself() {
    presenter->worker_proposes_itself_in_election(id);
    neighbour->assign_message(new ElectionProposal(id));
}

void Worker::end_election(Elected* elected) {
    if (elected->id == id) {
        presenter->election_is_finished(id);
    }
    else {
        presenter->worker_stops_election_participation(id);
        participates_in_election = false;

        neighbour->assign_message(new Elected(elected->id));
    }
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"
#include <tuple>
#include <cmath>

// sleep is needed since there is another thread
#define sleep() this_thread::sleep_for(chrono::milliseconds(25))

TEST_CASE(
    "Worker interacts with its neighbour and implements the Chang and Roberts algorithm for elections", 
    "[worker][message_buffer][messages]"
) {
    tuple<unsigned int, unsigned int> ids{GENERATE(
        tuple<unsigned int, unsigned int>{2, 8}, 
        tuple<unsigned int, unsigned int>{9, 5}
    )};
    unsigned int dummy_id{get<0>(ids)};
    unsigned int worker_id{get<1>(ids)};

    Worker dummy_worker(dummy_id, 0, nullptr);
    Worker worker(worker_id, 0, nullptr, &dummy_worker);

    thread worker_thread{ref(worker)};
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE(worker.is_running());
    
    SECTION("Worker is able to start election") {
        worker.assign_message(new StartElection());
        sleep();

        CHECK(worker.participates_in_election);

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::ElectionProposal);
        CHECK(message->cast_to<ElectionProposal>()->id == worker_id);

        delete message;
    }

    SECTION("Worker is able to participate in election") {
        worker.participates_in_election = false;
        worker.is_leader = GENERATE(true, false);

        worker.assign_message(new ElectionProposal(dummy_id));
        sleep();

        CHECK(worker.participates_in_election);
        CHECK_FALSE(worker.is_leader);

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::ElectionProposal);
        CHECK(message->cast_to<ElectionProposal>()->id == max(worker_id, dummy_id));

        delete message;
    }

    SECTION("Worker can handle out of order election proposals") {
        worker.participates_in_election = true;

        worker.assign_message(new ElectionProposal(dummy_id));
        sleep();

        CHECK(worker.participates_in_election);

        if (dummy_id > worker_id) {
            auto message{dummy_worker.message_buffer.take()};
            REQUIRE(message->type == MessageType::ElectionProposal);
            CHECK(message->cast_to<ElectionProposal>()->id == dummy_id);

            delete message;
        }
        else {
            REQUIRE(dummy_worker.message_buffer.is_empty());
        }
    }

    SECTION("Worker can be elected") {
        worker.participates_in_election = true;
        worker.is_leader = false;

        worker.assign_message(new ElectionProposal(worker_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK(worker.is_leader);

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::Elected);
        CHECK(message->cast_to<Elected>()->id == worker_id);

        delete message;
    }

    SECTION("Worker acts accordingly when someone is elected") {
        worker.participates_in_election = true;
        worker.is_leader = false;

        worker.assign_message(new Elected(dummy_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK_FALSE(worker.is_leader);

        auto message{dummy_worker.message_buffer.take()};
        REQUIRE(message->type == MessageType::Elected);
        CHECK(message->cast_to<Elected>()->id == dummy_id);

        delete message;
    }

    SECTION("Worker is able to finish election") {
        worker.participates_in_election = false;
        worker.is_leader = true;

        worker.assign_message(new Elected(worker_id));
        sleep();

        CHECK_FALSE(worker.participates_in_election);
        CHECK(worker.is_leader);

        CHECK(dummy_worker.message_buffer.is_empty());
    }

    worker.assign_message(new Stop());
    sleep();

    REQUIRE_FALSE(dummy_worker.is_running());
    REQUIRE_FALSE(worker.is_running());

    worker_thread.join();
}

#endif
