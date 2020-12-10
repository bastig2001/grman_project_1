#include "worker.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

using namespace std;


void Worker::assign_message(Message* message) {
    message_buffer.assign(message);
}

void Worker::set_neighbour(Worker* neighbour) {
    this->neighbour = neighbour;
}

void Worker::operator()() {
    current_neighbour = neighbour;
    if (current_neighbour) {

        bool continue_operation{true};
        while (continue_operation) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue_operation = act_upon_message(message_buffer.take());
        }
    }
}

ContinueOperation Worker::act_upon_message(Message* message) {
    ContinueOperation continue_operation{true};

    spdlog::info("Worker {} got following message: {}.", id, (string)*message);

    switch (message->type) {
        case MessageType::LogMessage:
            spdlog::info("Worker {} says:\n{}", id, message->cast_to<LogMessage>()->content);
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
    spdlog::info("Worker {} starts an election.", id);
    propose_oneself();
}

void Worker::participate_in_election(ElectionProposal* proposal) {
    is_leader = false;

    if (proposal->id > id) {
        forward_election_proposal(proposal);
    }
    else if (proposal->id == id) {
        be_elected();
    }
    else {
        if (participates_in_election) {
            discard_election_proposal(proposal);
        }
        else {
            propose_oneself();
        }
    }
}

void Worker::forward_election_proposal(ElectionProposal* proposal) {
    spdlog::info("Worker {} particpates in the election and forwards {}", id, proposal->id);
    participates_in_election = true;
    current_neighbour->assign_message(new ElectionProposal(proposal->id));
}

void Worker::be_elected() {
    spdlog::info("Worker {} has been elected.", id);
    participates_in_election = false;
    is_leader = true;
    current_neighbour->assign_message(new Elected(id));
}

void Worker::discard_election_proposal(ElectionProposal* proposal) {
    spdlog::info("Worker {} discards the election proposal for {}", proposal->id);
}

void Worker::propose_oneself() {
    spdlog::info("Worker {} proposes itself as leader.", id);
    participates_in_election = true;
    current_neighbour->assign_message(new ElectionProposal(id));
}

void Worker::end_election(Elected* elected) {
    if (elected->id == id) {
        spdlog::info("The election is over. Worker {} is the leader.", id);
    }
    else {
        participates_in_election = false;
        current_neighbour->assign_message(new Elected(elected->id));
    }
}
