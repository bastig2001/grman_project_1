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
    lock_guard<mutex> running_status_lck{running_status_mtx};
    running = false;

    if (command_line_thread.joinable()) {
        command_line_thread.join();
    }

    quitted.notify_all();
}

void CommandLine::wait() {
    unique_lock<mutex> running_status_lck{running_status_mtx};
    quitted.wait(running_status_lck, [this](){ return !running; });
}

void CommandLine::operator()() {
    struct termios original_stdin_settings{}, new_stdin_settings{};

    // save stdin settings
    tcgetattr(fileno(stdin), &original_stdin_settings); 

    new_stdin_settings = original_stdin_settings;

    // disbale echoing in new stdin settings
    new_stdin_settings.c_lflag &= (~ICANON & ~ECHO); 

    // load new stdin settings
    tcsetattr(fileno(stdin), TCSANOW, &new_stdin_settings); 

    while (running) {
        fd_set set{};
        struct timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(fileno(stdin), &set);

        int read_readiness{
            select(fileno(stdin) + 1, &set, nullptr, nullptr, &tv)
        };
        if (read_readiness > 0) {
            char input_char{};
            read(fileno(stdin), &input_char, 1); // read from stdin

            handle_input_key(input_char);
        }
    }

    // load back original stdin settings
    tcsetattr(fileno(stdin), TCSANOW, &original_stdin_settings);
}

void CommandLine::handle_input_key(char input_char) {
    if (in_esc_mode) {
        handle_input_key_in_esc_mode(input_char);        
    }
    else {
        handle_input_key_in_regular_mode(input_char);
    }
}

void CommandLine::handle_input_key_in_esc_mode(char input_char) {
    in_esc_mode = false;

    ctrl_sequence += input_char;
    if (ctrl_sequence == "[") { // CSI
        // expecting more keys
        in_esc_mode = true;
    }
    else if (ctrl_sequence == "[C") { // arrow key right
        move_cursor_right();
    }
    else if (ctrl_sequence == "[D") { // arrow key left
        move_cursor_left();
    }
    else if (ctrl_sequence == "[3") { // precursor to DEL
        // expecting more keys
        in_esc_mode = true;
    }
    else if (ctrl_sequence == "[3~") { // DEL
        delete_char_on_cursor();
    }
}

void CommandLine::handle_input_key_in_regular_mode(char input_char) {
    switch (input_char) {
        case 27: // ESC
            in_esc_mode = true;
            ctrl_sequence = "";
            break;
        case 127: // Backspace
            delete_char_before_cursor();
            break;
        case '\n':
            handle_newline();
            break;
        default:
            write_char(input_char);
            break;
    }
}

void CommandLine::move_cursor_right() {
    if (user_cursor_position < user_input.size()) {
        lock_guard<mutex> output_lock{output_mtx};

        user_cursor_position++;
        cout << "\33[C" << flush;
    }
}

void CommandLine::move_cursor_left() {
    if (user_cursor_position > 0) {
        lock_guard<mutex> output_lock{output_mtx};

        user_cursor_position--;
        cout << "\33[D" << flush;
    }
}

void CommandLine::delete_char_on_cursor() {
    if (user_cursor_position < user_input.size()) {
        lock_guard<mutex> output_lock{output_mtx};

        user_input.erase(user_cursor_position, 1);
        write_user_input_new(user_cursor_position);
    }
}

void CommandLine::delete_char_before_cursor() {
    if (user_cursor_position > 0) {
        lock_guard<mutex> output_lock{output_mtx};

        unsigned int char_to_remove_index{user_cursor_position - 1};
        user_cursor_position--;
        user_input.erase(char_to_remove_index, 1);
        write_user_input_new(char_to_remove_index);
    }
}

void CommandLine::handle_newline() {
    unique_lock<mutex> output_lock{output_mtx};

    string command{user_input};
    user_input = "";
    user_cursor_position = 0;

    cout << "\n> " << flush;

    output_lock.unlock();

    execute_command(command);
}

void CommandLine::write_char(char output_char) {
    lock_guard<mutex> output_lock{output_mtx};

    user_cursor_position++;

    if (user_cursor_position < user_input.size() - 1) {
        user_input.insert(
            user_input.begin() + user_cursor_position - 1, 
            output_char
        );
        write_user_input_new(user_cursor_position - 1);
    }
    else {
        user_input += output_char;
        cout << output_char << flush;
    }
}

void CommandLine::write_user_input_new(unsigned int index) {
    cout << "\r\33[" << promp_length + index << "C"                // move to position of first char to change
         << "\33[K"                                                // clear all chars to right of cursor
         << user_input.substr(index)                               // new user input
         << "\r\33[" << promp_length + user_cursor_position << "C" // move cursor to previous position
         << flush; 
}

void CommandLine::execute_command(const std::string&) {
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
    cout << "> " << user_input                                     // prompt and user input
         << "\r\33[" << promp_length + user_cursor_position << "C" // move cursor to previous position
         << flush;
}
