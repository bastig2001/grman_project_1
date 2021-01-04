#include "worker.h"
#include "messages.h"
#include "presenters/no_presenter.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace std;


void Worker::assign_message_sync(Message* message) {
    message_buffer.assign_sync(message);
}

void Worker::assign_message_async(Message* message) {
    message_buffer.assign_async(message);
}

void Worker::set_neighbours(vector<Worker*> neighbours) {
    if (neighbours.size() > 0) {
        this->neighbours = neighbours;
    }
    else {
        throw invalid_argument("There must be at least on neighbour.");
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
    if (neighbours.size() > 0) {
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
        case MessageType::DeadWorker:
            remove_dead_worker(message->cast_to<DeadWorker>());
            break;
        case MessageType::NewWorker:
            add_new_worker(message->cast_to<NewWorker>());
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
    send_to_neighbour(new ElectionProposal(*proposal));
}

void Worker::be_elected() {
    presenter->worker_is_elected(id);
    is_leader = true;

    presenter->worker_stops_election_participation(id);
    participates_in_election = false;

    send_to_neighbour(new Elected(id));
}

void Worker::propose_oneself() {
    presenter->worker_proposes_itself_in_election(id);
    send_to_neighbour(new ElectionProposal(id));
}

void Worker::end_election(Elected* elected) {
    if (elected->id == id) {
        presenter->election_is_finished(id);
    }
    else {
        presenter->worker_stops_election_participation(id);
        participates_in_election = false;

        send_to_neighbour(new Elected(elected->id));
    }
}

void Worker::remove_dead_worker(DeadWorker* dead_worker) {
    if (position_is_not_neighbour(dead_worker->position)) {
        neighbours.erase(
            neighbours.begin() 
                + 
            get_neighbours_index_for_position(dead_worker->position)
        );

        send_to_neighbour(new DeadWorker(*dead_worker));
    }
    // When the position is its neighbour, 
    // the dead worker has been already removed, 
    // because only the preceding neighbour can know if a worker is dead 
    // and must therefore be the first to recognize and remove it.
}

bool Worker::position_is_not_neighbour(unsigned int position) {
    return !(
        position 
            == 
        (this->position + 1) % (neighbours.size() + 1)
    );
}

void Worker::add_new_worker(NewWorker* new_worker) {
    unsigned int new_neighbour_index{
        get_neighbours_index_for_position(new_worker->position)
    };

    if (*neighbours[new_neighbour_index] != *new_worker->worker) {
        neighbours.insert(
            neighbours.begin() + new_neighbour_index, 
            new_worker->worker
        );

        send_to_neighbour(new NewWorker(*new_worker));
    }
    // If the new worker and the neighbour at the index for the new worker 
    // are the same, the new worker has already been inserted 
    // and there is nothing left to do
}

unsigned int Worker::get_neighbours_index_for_position(unsigned int position) {
    if (position > this->position) {
        return position - this->position - 1;
    }
    else {
        return position - this->position + neighbours.size();
    }
}

void Worker::send_to_neighbour(Message* message) {
    neighbours[0]->assign_message_sync(message);
}

bool Worker::operator==(const Worker& other_worker) {
    return id == other_worker.id;
}

bool Worker::operator!=(const Worker& other_worker) {
    return !(*this == other_worker);
}
