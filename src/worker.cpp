#include "worker.h"
#include "presenters/no_presenter.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace std;


void Worker::assign_message(Message* message) {
    message_buffer.assign(message);
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

Worker::~Worker() {
    // if the presenter of the worker is no presenter it needs to be deleted
    NoPresenter* no_presenter{dynamic_cast<NoPresenter*>(presenter)};
    if (no_presenter) {
        delete no_presenter;
    }
}

void Worker::operator()() {
    if (neighbour) {
        bool continue_operation{true};
        while (continue_operation) {
            this_thread::sleep_for(sleeptime);
            continue_operation = act_upon_message(message_buffer.take());
        }
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
