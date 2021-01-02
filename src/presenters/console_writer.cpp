#include "presenters/console_writer.h"

#include <fmt/color.h>
#include <iostream>

using namespace std;
using namespace fmt;


void ConsoleWriter::ring_started() {
    if (logs_to_file()) {
        Logger::ring_started();
    }

    print("The Ring has started.\n");
}

void ConsoleWriter::ring_stopped() {
    if (logs_to_file()) {
        Logger::ring_stopped();
    }

    print(fg(color::red), "The Ring has stopped.\n");
}

void ConsoleWriter::worker_says(unsigned int worker_id, const string& message) {
    if (logs_to_file()) {
        Logger::worker_says(worker_id, message);
    }

    print(
        "Worker {} says: {}\n", 
        worker_id, 
        format(emphasis::italic, message)
    );
}

void ConsoleWriter::worker_starts_election(unsigned int worker_id) {
    if (logs_to_file()) {
        Logger::worker_starts_election(worker_id);
    }

    print(
        "Worker {} {}.\n", 
        worker_id, 
        format(emphasis::bold, "starts an election")
    );
}

void ConsoleWriter::worker_proposes_itself_in_election(unsigned int worker_id) {
    if (logs_to_file()) {
        Logger::worker_proposes_itself_in_election(worker_id);
    }

    print(
        "Worker {} {} itself as leader.\n", 
        format(fg(color::green_yellow), "{}", worker_id), 
        format(emphasis::underline, "proposes")
    );
}

void ConsoleWriter::worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    if (logs_to_file()) {
        Logger::worker_forwards_election_proposal(worker_id, proposal_id);
    }

    print(
        "Worker {} {} the election proposal for Worker {}.\n", 
        worker_id, 
        format(emphasis::underline, "forwards"), 
        format(fg(color::light_blue), "{}", proposal_id)
    );
}

void ConsoleWriter::worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    if (logs_to_file()) {
        Logger::worker_discards_election_proposal(worker_id, proposal_id);
    }

    print(
        "Worker {} {} the election proposal for Worker {}.\n",
        worker_id, 
        format(emphasis::underline, "discards"), 
        format(fg(color::dark_red), "{}", proposal_id)
    );
}

void ConsoleWriter::worker_is_elected(unsigned int worker_id) {
    if (logs_to_file()) {
        Logger::worker_is_elected(worker_id);
    }

    print(
        "Worker {} {}.\n",
        format(fg(color::green), "{}", worker_id), 
        format(emphasis::underline | fg(color::dark_green), "has been elected")
    );
}

void ConsoleWriter::election_is_finished(unsigned int leader_id) {
    if (logs_to_file()) {
        Logger::election_is_finished(leader_id);
    }

    print(
        fg(color::gold),
        "The election is finished. {} is the leader.\n",
        format(
            fg(color::green) | emphasis::underline | emphasis::bold, 
            "Worker {}", leader_id
        )
    );
}

ConsoleWriter::~ConsoleWriter() {
    print("\n"); // print and flush nothing just to reset cursor colour
}
