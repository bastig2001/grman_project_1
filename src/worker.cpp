#include "worker.h"
#include "message.h"
#include "presenter.h"

#include <chrono>
#include <stdexcept>
#include <thread>

using namespace std;


bool Worker::assign_message_and_wait(Message* message) {
    // one worker sleeptime should be expected at least,
    // two, because it might get a message from the Ring,
    // 2.5, because there are processes besides just sleeping,
    // the waittime is at least 1s
    return message_buffer.assign_and_wait(
        message, 
        max(1000u, (unsigned int)(sleeptime * 2.5))
    );
}

void Worker::assign_message(Message* message) {
    message_buffer.assign(message);
}

void Worker::set_colleagues(vector<Worker*> colleagues) {
    if (colleagues.size() > 0) {
        this->colleagues = colleagues;
    }
    else {
        throw invalid_argument(
            "There must be at least on colleagues to which to send."
        );
    }
}

bool Worker::is_running() const {
    return running;
}

void Worker::operator()() {
    if (colleagues.size() > 0) {
        running = true;

        bool continue_operation{true};
        while (continue_operation) {
            this_thread::sleep_for(chrono::milliseconds(sleeptime));
            continue_operation = act_upon_message(message_buffer.take());
        }

        running = false;
    }
}

ContinueOperation Worker::act_upon_message(Message* message) {
    ContinueOperation continue_operation{true};

    presenter->show(CreateEvent::got_message(id, message));

    switch (message->type) {
        case MessageType::LogMessage:
            presenter->show(
                CreateEvent::says(id, message->cast_to<LogMessage>()->content)
            );
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
            handle_dead_worker(message->cast_to<DeadWorker>());
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
    presenter->show(CreateEvent::election_started(id));

    presenter->show(CreateEvent::participates(id));
    participates_in_election = true;

    propose_oneself();
}

void Worker::participate_in_election(ElectionProposal* proposal) {
    if (is_leader) {
        presenter->show(CreateEvent::resigned(id));
        is_leader = false;
    }

    bool already_participated_in_election{participates_in_election};

    if (!participates_in_election) {
        participates_in_election = true;
        presenter->show(CreateEvent::participates(id));
    }

    if (proposal->id > id) {
        forward_election_proposal(proposal);
    }
    else if (proposal->id == id) {
        be_elected();
    }
    else {
        if (already_participated_in_election) {
            presenter->show(CreateEvent::proposal_discarded(id, proposal->id));
        }
        else {
            propose_oneself();
        }
    }
}

void Worker::forward_election_proposal(ElectionProposal* proposal) {
    presenter->show(CreateEvent::proposal_forwarded(id, proposal->id));
    send_to_neighbour(new ElectionProposal(*proposal));
}

void Worker::be_elected() {
    presenter->show(CreateEvent::is_elected(id));
    is_leader = true;

    presenter->show(CreateEvent::participation_stopped(id));
    participates_in_election = false;

    send_to_neighbour(new Elected(id));
}

void Worker::propose_oneself() {
    presenter->show(CreateEvent::proposed_themselves(id));
    send_to_neighbour(new ElectionProposal(id));
}

void Worker::end_election(Elected* elected) {
    if (elected->id == id) {
        presenter->show(CreateEvent::election_finished(id));
    }
    else {
        presenter->show(CreateEvent::participation_stopped(id));
        participates_in_election = false;

        send_to_neighbour(new Elected(elected->id));
    }
}

void Worker::handle_dead_worker(DeadWorker* dead_worker) {
    if (dead_worker->position != get_direct_neighbour_position()) {
        remove_dead_worker(dead_worker->position);
    }
    // When the position is its neighbour, 
    // the dead worker has been already removed, 
    // because only the preceding neighbour can know if a worker is dead 
    // and must therefore be the first to recognize and remove it.
}

void Worker::add_new_worker(NewWorker* new_worker) {
    unsigned int new_neighbour_index{
        get_neighbours_index_for_position(new_worker->position)
    };

    if (*colleagues[new_neighbour_index] != *new_worker->worker) {
        presenter->show(
            CreateEvent::colleague_added(id, new_worker->worker->id)
        );

        colleagues.insert(
            colleagues.begin() + new_neighbour_index, 
            new_worker->worker
        );

        if (new_worker->position <= position) {
            position++; // update position when necessary
        }

        send_to_neighbour(new NewWorker(*new_worker));
    }
    // If the new worker and the neighbour at the index for the new worker 
    // are the same, the new worker has already been inserted 
    // and there is nothing left to do
}

void Worker::send_to_neighbour(Message* message) {
    if (previous_message_sent.valid() && !previous_message_sent.get()) {
        // previous message still hasn't been retrieved by neighbour,
        // neighbour is considered dead
        unsigned int neighbour_position{get_direct_neighbour_position()};
        presenter->show(
            CreateEvent::dead_neighbour_recognized(
                id, 
                colleagues[neighbour_position]->id
            )
        );
        remove_dead_worker(neighbour_position);
    }

    previous_message_sent = async(
        launch::async,
        [message{message}, this](){ 
            return colleagues[0]->assign_message_and_wait(message); 
        }
    );
}

void Worker::remove_dead_worker(unsigned int position) {
    presenter->show(
        CreateEvent::colleague_removed(id, colleagues[position]->id)
    );

    colleagues.erase(
        colleagues.begin() 
            + 
        get_neighbours_index_for_position(position)
    );

    if (position < this->position) {
        this->position--; // update position when necessary
    }

    send_to_neighbour(new DeadWorker(position));
}

unsigned int Worker::get_direct_neighbour_position() {
    return (position + 1) % colleagues.size();
}

unsigned int Worker::get_neighbours_index_for_position(unsigned int position) {
    if (position > this->position) {
        return position - this->position - 1;
    }
    else {
        return position - this->position - 1 + colleagues.size();
    }
}

bool Worker::operator==(const Worker& other_worker) {
    return id == other_worker.id;
}

bool Worker::operator!=(const Worker& other_worker) {
    return !(*this == other_worker);
}
