#include "presenters/command_line.h"
#include "presenters/no_presenter.h"

using namespace std;


CommandLine::~CommandLine() {
    // if the output writer is no presenter it needs to be deleted
    NoPresenter* no_presenter{dynamic_cast<NoPresenter*>(output_writer)};
    if (no_presenter) {
        delete no_presenter;
    }
}

void CommandLine::set_output_writer(Presenter* output_writer) {
    // sets the output writer to the given presenter 
    // when it's not a null pointer,
    // otherwise it's set to no presenter
    this->output_writer = 
        output_writer
        ? output_writer
        : new NoPresenter();
}

void CommandLine::log(spdlog::level::level_enum log_level, const string& message) {
    output_writer->log(log_level, message);
}

void CommandLine::ring_created(size_t ring_size) {
    output_writer->ring_created(ring_size);
}

void CommandLine::ring_starts() {
    output_writer->ring_starts();
}

void CommandLine::ring_started() {
    output_writer->ring_started();
}

void CommandLine::ring_stops() {
    output_writer->ring_stops();
}

void CommandLine::ring_stopped() {
    output_writer->ring_stopped();
}

void CommandLine::worker_created(unsigned int worker_id, unsigned int position) {
    output_writer->worker_created(worker_id, position);
}

void CommandLine::worker_started(unsigned int position) {
    output_writer->worker_started(position);
}

void CommandLine::worker_stopped(unsigned int position) {
    output_writer->worker_stopped(position);
}

void CommandLine::worker_got_message(unsigned int worker_id, Message* message) {
    output_writer->worker_got_message(worker_id, message);
}

void CommandLine::worker_says(unsigned int worker_id, const string& message) {
    output_writer->worker_says(worker_id, message);
}

void CommandLine::worker_starts_election(unsigned int worker_id) {
    output_writer->worker_starts_election(worker_id);
}

void CommandLine::worker_participates_in_election(unsigned int worker_id) {
    output_writer->worker_participates_in_election(worker_id);
}

void CommandLine::worker_proposes_itself_in_election(unsigned int worker_id) {
    output_writer->worker_proposes_itself_in_election(worker_id);
}

void CommandLine::worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    output_writer->worker_forwards_election_proposal(worker_id, proposal_id);
}

void CommandLine::worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    output_writer->worker_discards_election_proposal(worker_id, proposal_id);
}

void CommandLine::worker_stops_election_participation(unsigned int worker_id) {
    output_writer->worker_stops_election_participation(worker_id);
}

void CommandLine::worker_is_elected(unsigned int worker_id) {
    output_writer->worker_is_elected(worker_id);
}

void CommandLine::worker_resigns_as_leader(unsigned int worker_id) {
    output_writer->worker_resigns_as_leader(worker_id);
}

void CommandLine::election_is_finished(unsigned int leader_id) {
    output_writer->election_is_finished(leader_id);
}

void CommandLine::worker_recognizes_dead_neighbour(unsigned int worker_id, unsigned int neighbour_position) {
    output_writer->worker_recognizes_dead_neighbour(worker_id, neighbour_position);
}

void CommandLine::worker_removes_neighbour(unsigned int worker_id, unsigned int neighbour_position) {
    output_writer->worker_removes_neighbour(worker_id, neighbour_position);
}

void CommandLine::worker_adds_neighbour(unsigned int worker_id, unsigned int neighbour_position) {
    output_writer->worker_adds_neighbour(worker_id, neighbour_position);
}
