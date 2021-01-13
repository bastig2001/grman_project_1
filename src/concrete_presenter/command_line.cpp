#include "concrete_presenter.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>

using namespace std;


void ConcretePresenter::start_command_line() {
    if (ring) {
        lock_guard<mutex> start_lock{running_mtx};

        running = true;
        cout << prompt << flush;
        pre_output = [this](){ clear_line(); };
        post_output = [this](){ print_prompt_and_user_input(); };
        define_command_parser();

        command_line_thread = thread{ref(*this)};
    }
    else {
        throw invalid_argument("ConcretePresenter needs a pointer to a Ring.");
    }
}

void ConcretePresenter::stop_command_line() {
    if (running) {
        exit();
    }

    if (command_line_thread.joinable()) {
        command_line_thread.join();
    }
}

void ConcretePresenter::exit() {
    lock_guard<mutex> stop_lock{running_mtx};

    running = false;
    pre_output = [](){};
    post_output = [](){};

    println("");

    exited.notify_all();
}

void ConcretePresenter::wait_for_exit() {
    unique_lock<mutex> wait_lock{running_mtx};
    exited.wait(wait_lock, [this](){ return !running; });
}

void ConcretePresenter::operator()() {
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

void ConcretePresenter::handle_input_key(char input_char) {
    if (in_esc_mode) {
        handle_input_key_in_esc_mode(input_char);        
    }
    else {
        handle_input_key_in_regular_mode(input_char);
    }
}

void ConcretePresenter::handle_input_key_in_esc_mode(char input_char) {
    in_esc_mode = false;

    ctrl_sequence += input_char;
    if (ctrl_sequence == "[") { // CSI
        // expecting more keys
        in_esc_mode = true;
    }
    else if (ctrl_sequence == "[A") { // arrow key up
        show_history_up();
    }
    else if (ctrl_sequence == "[B") { // arrow key down
        show_history_down();
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
        do_delete();
    }
}

void ConcretePresenter::handle_input_key_in_regular_mode(char input_char) {
    switch (input_char) {
        case 27: // ESC
            in_esc_mode = true;
            ctrl_sequence = "";
            break;
        case 127: // Backspace
            do_backspace();
            break;
        case '\n':
            handle_newline();
            break;
        case 4: // ^D ... Ctrl + D
            exit();
            break;
        default:
            write_char(input_char);
            break;
    }
}

void ConcretePresenter::show_history_up() {
    if (next_input_history_index < input_history.size()) {
        if (next_input_history_index == 0) {
            original_input = current_input;
        }

        update_input(input_history[next_input_history_index]);

        next_input_history_index++;
    }
}

void ConcretePresenter::show_history_down() {
    if (next_input_history_index > 1) {
        next_input_history_index--;

        update_input(input_history[next_input_history_index - 1]);
    }
    else if (next_input_history_index == 1) {
        next_input_history_index = 0;

        update_input(original_input);
    }
}

void ConcretePresenter::update_input(const string& input) {
    lock_guard<mutex> output_lock{output_mtx};

    current_input = input;
    cursor_position = min(cursor_position, current_input.size());

    write_user_input();
}

void ConcretePresenter::move_cursor_right() {
    if (cursor_position < current_input.size()) {
        lock_guard<mutex> output_lock{output_mtx};

        cursor_position++;
        cout << "\33[C" << flush;
    }
}

void ConcretePresenter::move_cursor_left() {
    if (cursor_position > 0) {
        lock_guard<mutex> output_lock{output_mtx};

        cursor_position--;
        cout << "\33[D" << flush;
    }
}

void ConcretePresenter::do_delete() {
    if (cursor_position < current_input.size()) {
        lock_guard<mutex> output_lock{output_mtx};

        current_input.erase(cursor_position, 1);
        write_user_input(cursor_position);
    }
}

void ConcretePresenter::do_backspace() {
    if (cursor_position > 0) {
        lock_guard<mutex> output_lock{output_mtx};

        unsigned long char_to_remove_index{cursor_position - 1};
        cursor_position--;
        current_input.erase(char_to_remove_index, 1);
        write_user_input(char_to_remove_index);
    }
}

void ConcretePresenter::handle_newline() {
    unique_lock<mutex> output_lock{output_mtx};

    update_input_history(current_input);
    string command{current_input};
    current_input = "";
    cursor_position = 0;

    cout << "\n> " << flush;

    output_lock.unlock();

    execute(command);
}

void ConcretePresenter::update_input_history(const string& input) {
    next_input_history_index = 0;

    if (input_history.size() == 0 || input != input_history[0]) {
        if (input_history.size() == max_input_history_size) {
            input_history.erase(input_history.end() - 1);
        }

        input_history.insert(input_history.begin(), input);
    }
}

void ConcretePresenter::write_char(char output_char) {
    lock_guard<mutex> output_lock{output_mtx};

    cursor_position++;

    if (cursor_position < current_input.size() - 1) {
        current_input.insert(
            current_input.begin() + cursor_position - 1, 
            output_char
        );
        write_user_input(cursor_position - 1);
    }
    else {
        current_input += output_char;
        cout << output_char << flush;
    }
}

void ConcretePresenter::write_user_input(unsigned int start_index) {
    cout << "\r\33[" << prompt_length + start_index << "C"     // move to position of first char to change
         << "\33[K"                                            // clear all chars to right of cursor
         << current_input.substr(start_index)                  // new user input
         << "\r\33[" << prompt_length + cursor_position << "C" // move cursor to previous position
         << flush; 
}

void ConcretePresenter::clear_line() {
    cout << "\33[2K\r" << flush;
}

void ConcretePresenter::print_prompt_and_user_input() {
    cout << prompt << current_input                            // prompt and user input
         << "\r\33[" << prompt_length + cursor_position << "C" // move cursor to previous position
         << flush;
}
