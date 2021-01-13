#pragma once

#include "presenter.h"
#include "ring.h"

#include <fmt/color.h>
#include <peglib.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
#include <vector>


// The conrete implementation of Presenter used as the user interface
class ConcretePresenter: public Presenter {
  private:
    const std::string prompt{"> "};
    const unsigned long prompt_length{prompt.length()};

    Ring* ring{};

    /* logging, implemented in concrete_presenter/presenter.cpp */
    std::shared_ptr<spdlog::logger> console_logger;
    std::shared_ptr<spdlog::logger> file_logger;

    void set_log_to_console();
    std::function<void(spdlog::level::level_enum, const std::string&)> 
        log_to_console{[](spdlog::level::level_enum, const std::string&){}};

    /* command line, implemented in concrete_presenter/command_line.cpp */
    bool running{false};
    std::thread command_line_thread;
    std::mutex running_mtx;
    std::condition_variable exited;

    const unsigned int max_input_history_size{100};
    std::vector<std::string> input_history{};
    unsigned int next_input_history_index{0};
    std::string current_input{""};
    std::string original_input{};

    std::string ctrl_sequence{};
    bool in_esc_mode{false};
    unsigned long cursor_position{0};

    void handle_input_key(char input_char);
    void handle_input_key_in_esc_mode(char input_char);
    void handle_input_key_in_regular_mode(char input_char);
    void show_history_up();
    void show_history_down();
    void update_input(const std::string& input);
    void move_cursor_right();
    void move_cursor_left();
    void do_delete();
    void do_backspace();
    void handle_newline();
    void update_input_history(const std::string& input);
    void write_char(char output_char);
    void write_user_input(unsigned int start_index = 0);

    /* command execution, implemented in concrete_presenter/executor.cpp */
    peg::parser command_parser{};
    void define_command_parser();

    void execute(const std::string& command);

    void print_help();
    void list_workers();
    void start_election(const peg::SemanticValues& values);
    void stop_worker(const peg::SemanticValues& values);
    void start_worker(const peg::SemanticValues& values);
    void restart_ring();
    void print_error(size_t column, const std::string& err_msg);
    void exit();

    /* event presentation, implemented in concrete_presenter/presenter.cpp */
    void ring_started();
    void ring_stopped();
    void says(Says* event);
    void election_started(WorkerEvent* event);
    void proposed_themselves(WorkerEvent* event);
    void is_elected(WorkerEvent* event);
    void election_finished(WorkerEvent* event);
    void proposal_forwarded(ProposalEvent* event);
    void proposal_discarded(ProposalEvent* event);
    void dead_neighbour_recognized(ColleagueEvent* event);

    /* output, implemented in concrete_presenter/command_line.cpp */
    std::mutex output_mtx;

    template<typename... Args>
    void println(const Args&... args) {
        std::lock_guard<std::mutex> output_lock{output_mtx};
        pre_output();
        fmt::print(args...);
        fmt::print("\n");
        post_output();
    }

    template<typename... Args>
    void eprintln(const Args&... args) {
        std::lock_guard<std::mutex> output_lock{output_mtx};
        pre_output();
        (std::cerr << ... << args) << std::endl;
        post_output();
    }

    void clear_line();
    void print_prompt_and_user_input();

    std::function<void()> pre_output{[](){}};
    std::function<void()> post_output{[](){}};

  public:
    ConcretePresenter(
        std::shared_ptr<spdlog::logger> console_logger,
        std::shared_ptr<spdlog::logger> file_logger
    ): console_logger{console_logger},
       file_logger{file_logger}
    {
        set_log_to_console();
    }

    /* implemented in concrete_presenter/presenter.cpp */
    ~ConcretePresenter();

    // ring needs to be set before starting the command line,
    // throws invalid_argument when the given pointer is a null_ptr
    void set_ring(Ring* ring);

    // shows to the user and/or logs the given event
    void show(Event* event) override;

    // logs with both loggers
    void log(spdlog::level::level_enum level, const std::string& message);

    /* implemented in concrete_presenter/command_line.cpp */
    // starts/stops the command line in a different thread,
    // throws invalid_argument when the ring hasn't been set
    void start_command_line();
    void stop_command_line();

    // waits for the command line to exit
    void wait_for_exit();

    // the method which implements the command line
    void operator()();
};

