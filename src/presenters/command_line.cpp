#include "presenters/command_line.h"
#include "presenters/no_presenter.h"

#include "peglib.h"
#include <any>
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
    exit();

    if (command_line_thread.joinable()) {
        command_line_thread.join();
    }
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
        case 4: // ^D
            exit();
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

using namespace peg;

void CommandLine::define_command_parser() {
    command_parser = (R"(
        Command       <- Help / List / Exit / StartElection / Stop / Start
        Help          <- 'h'i / 'help'i
        List          <- 'show'i / 'list'i / 'ls'
        Exit          <- 'q'i / 'quit'i / 'exit'i
        StartElection <- 'start-election'i Pos?
        Stop          <- 'stop'i Pos+
        Start         <- 'start'i Pos+
        Pos           <- Number
        Number        <- < [0-9]+ >

        %whitespace   <- [ \t]*
    )");

    command_parser.log = 
        [this](size_t, size_t col, const string& msg) { 
            print_error(col, msg); 
        };
    command_parser["Help"] = 
        [this](const SemanticValues&){ print_help(); };
    command_parser["List"] = 
        [this](const SemanticValues&){ list_workers(); };
    command_parser["Exit"] = 
        [this](const SemanticValues&){ exit(); };
    command_parser["StartElection"] = 
        [this](const SemanticValues& values){ start_election(values); };
    command_parser["Stop"] = 
        [this](const SemanticValues& values){ stop_workers(values); };
    command_parser["Start"] = 
        [this](const SemanticValues& values){ start_workers(values); };
    command_parser["Pos"] = 
        [](const SemanticValues& values){
            return any_cast<unsigned int>(values[0]);
        };
    command_parser["Number"] = 
        [](const SemanticValues& values){
            return values.token_to_number<unsigned int>();
        };
}

void CommandLine::execute_command(const string& command) {
    command_parser.parse(command.c_str());
}

void CommandLine::print_help() {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    cout << "Following Commands are available:\n"
         << "  h, help               outputs this help message\n"
         << "  ls, list, show        lists all Workers in the Ring\n"
         << "  q, quit, exit         exits the program\n"
         << "  start-election [POS]  starts an election with the Worker at the given position or at position 0\n"
         << "  stop POS ...          stops the Workers at the given positions\n"
         << "  start POS ...         starts the Workers at the given positions" 
         << endl; 
    print_prompt_and_user_input();
}

void CommandLine::list_workers() {
    lock_guard<mutex> output_lock{output_mtx};
    clear_line();
    cout << "List" << endl;
    print_prompt_and_user_input();
}

void CommandLine::exit() {
    lock_guard<mutex> running_status_lck{running_status_mtx};
    running = false;
    quitted.notify_all();
}

void CommandLine::start_election(const SemanticValues& values) {
    if (values.size() == 0) {
        ring->start_election();
    }
    else {
        if (!ring->start_election_at_position(
            any_cast<unsigned int>(values[0])
        )) {
            lock_guard<mutex> output_lock{output_mtx};
            clear_line();
            cerr << "There is no Worker at the given position" << endl;
            print_prompt_and_user_input();
        }
    }
}

void CommandLine::stop_workers(const SemanticValues&) {

}

void CommandLine::start_workers(const SemanticValues&) {

}

void CommandLine::print_error(size_t column, const string& err_msg) {
    lock_guard<mutex> output_lock{output_mtx};

    clear_line();

    string msg{err_msg + " starting here "};
    for (unsigned int i{0}; i < promp_length + column - 1; i++) {
        cerr << " ";
    }
    cerr << "^\n" << msg;
    for (unsigned long i{msg.size()}; i < promp_length + column - 1; i++) {
        cerr << "_";
    }
    if (msg.size() < column) {
        cerr << "|";
    }
    cerr << "\nRun help for more information." << endl;

    print_prompt_and_user_input();
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
