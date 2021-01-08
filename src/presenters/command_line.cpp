#include "presenters/command_line.h"
#include "presenters/no_presenter.h"

#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace std;


CommandLine::~CommandLine() {
    stop();
    delete output_writer;
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

void CommandLine::set_ring(Ring* ring) {
    this->ring = ring;
}

bool CommandLine::start() {
    if (ring) {
        cout << "> " << flush;
        running = true;
        command_line_thread = thread{ref(*this)};
        return true;
    }
    else {
        return false;
    }
}

void CommandLine::stop() {
    running = false;
    if (command_line_thread.joinable()) {
        command_line_thread.join();
    }
}

void CommandLine::operator()() {
    struct termios old_settings{}, new_settings{};
    tcgetattr(fileno(stdin), &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(fileno(stdin), TCSANOW, &new_settings);

    while (running) {
        fd_set set{};
        struct timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(fileno(stdin), &set);

        int res{select(fileno(stdin) + 1, &set, nullptr, nullptr, &tv)};
        if (res > 0) {
            char key{};
            read(fileno(stdin), &key, 1);

            if ((int)key == 127) { // if backspace
                input_buffer << "\b \b";

                lock_guard<mutex> output_lock{output_mtx};
                cout << "\b \b" << flush;
            }
            else {
                input_buffer << key;

                lock_guard<mutex> output_lock{output_mtx};
                cout << key << flush;
            }
        }
    }

    tcsetattr(fileno(stdin), TCSANOW, &old_settings);
}

void CommandLine::log(spdlog::level::level_enum log_level, const string& message) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->log(log_level, message);
    print_prompt_and_user_input();
}

void CommandLine::ring_created(size_t ring_size) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->ring_created(ring_size);
    print_prompt_and_user_input();
}

void CommandLine::ring_starts() {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->ring_starts();
    print_prompt_and_user_input();
}

void CommandLine::ring_started() {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->ring_started();
    print_prompt_and_user_input();
}

void CommandLine::ring_stops() {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->ring_stops();
    print_prompt_and_user_input();
}

void CommandLine::ring_stopped() {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->ring_stopped();
    print_prompt_and_user_input();
}

void CommandLine::worker_created(unsigned int worker_id, unsigned int position) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_created(worker_id, position);
    print_prompt_and_user_input();
}

void CommandLine::worker_started(unsigned int position) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_started(position);
    print_prompt_and_user_input();
}

void CommandLine::worker_stopped(unsigned int position) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_stopped(position);
    print_prompt_and_user_input();
}

void CommandLine::worker_got_message(unsigned int worker_id, Message* message) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_got_message(worker_id, message);
    print_prompt_and_user_input();
}

void CommandLine::worker_says(unsigned int worker_id, const string& message) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_says(worker_id, message);
    print_prompt_and_user_input();
}

void CommandLine::worker_starts_election(unsigned int worker_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_starts_election(worker_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_participates_in_election(unsigned int worker_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_participates_in_election(worker_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_proposes_itself_in_election(unsigned int worker_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_proposes_itself_in_election(worker_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_forwards_election_proposal(worker_id, proposal_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_discards_election_proposal(worker_id, proposal_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_stops_election_participation(unsigned int worker_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_stops_election_participation(worker_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_is_elected(unsigned int worker_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_is_elected(worker_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_resigns_as_leader(unsigned int worker_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_resigns_as_leader(worker_id);
    print_prompt_and_user_input();
}

void CommandLine::election_is_finished(unsigned int leader_id) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->election_is_finished(leader_id);
    print_prompt_and_user_input();
}

void CommandLine::worker_recognizes_dead_neighbour(unsigned int worker_id, unsigned int neighbour_position) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_recognizes_dead_neighbour(worker_id, neighbour_position);
    print_prompt_and_user_input();
}

void CommandLine::worker_removes_neighbour(unsigned int worker_id, unsigned int neighbour_position) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_removes_neighbour(worker_id, neighbour_position);
    print_prompt_and_user_input();
}

void CommandLine::worker_adds_neighbour(unsigned int worker_id, unsigned int neighbour_position) {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    output_writer->worker_adds_neighbour(worker_id, neighbour_position);
    print_prompt_and_user_input();
}

void CommandLine::clear_line() {
    cout << "\33[2K\r" << flush;
}

void CommandLine::print_prompt_and_user_input() {
    cout << "> " << input_buffer.str() << flush;
}
