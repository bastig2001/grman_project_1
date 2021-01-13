#pragma once

#include "fmt/color.h"
#include "presenter.h"
#include "ring.h"

#include "peglib.h"
#include "spdlog/common.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>


class ConcretePresenter: public Presenter {
  private:
    const std::string prompt{"> "};
    const unsigned long prompt_length{prompt.length()};

    Ring* ring{};

    std::shared_ptr<spdlog::logger> console_logger;
    std::shared_ptr<spdlog::logger> file_logger;

    void set_log_to_console();
    std::function<void(spdlog::level::level_enum, const std::string&)> 
        log_to_console{[](spdlog::level::level_enum, const std::string&){}};

    bool running{false};
    std::thread command_line_thread;
    std::mutex running_mtx;
    std::condition_variable exited;

    std::mutex output_mtx;
    std::string input{};
    std::string ctrl_sequence{};
    bool in_esc_mode{false};
    unsigned int cursor_position{0};

    void handle_input_key(char input_char);
    void handle_input_key_in_esc_mode(char input_char);
    void handle_input_key_in_regular_mode(char input_char);
    void move_cursor_right();
    void move_cursor_left();
    void do_delete();
    void do_backspace();
    void handle_newline();
    void write_char(char output_char);
    void write_user_input(unsigned int start_index);

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

    ~ConcretePresenter();

    void set_ring(Ring* ring);

    void show(Event* event) override;

    void log(spdlog::level::level_enum level, const std::string& message);

    void start_command_line();
    void stop_command_line();

    void wait_for_exit();

    void operator()();
};

