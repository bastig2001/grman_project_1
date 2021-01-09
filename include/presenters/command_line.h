#pragma once

#include "presenters/presenter.h"
#include "ring.h"

#include "peglib.h"
#include <thread>
#include <mutex>
#include <condition_variable>

enum class WorkerIdentifierType {
    Id,
    Pos
};

struct WorkerIdentifier {
    WorkerIdentifierType type{};
    unsigned int value{};
};

// A Presenter that provides a command line to interact with the Ring.
// Outputs are written with the given Presenter.
class CommandLine: public Presenter {
  private:
    Presenter* output_writer;
    Ring* ring;

    bool running{false};
    std::thread command_line_thread;
    std::mutex running_status_mtx;
    std::condition_variable quitted;

    std::mutex output_mtx;
    std::string user_input{};
    std::string ctrl_sequence{};
    bool in_esc_mode{false};
    unsigned int user_cursor_position{0};
    const unsigned int promp_length{2};
    peg::parser command_parser;

    void set_output_writer(Presenter* output_writer);
    void define_command_parser();

    void handle_input_key(char input_char);
    void handle_input_key_in_esc_mode(char input_char);
    void handle_input_key_in_regular_mode(char input_char);
    void move_cursor_right();
    void move_cursor_left();
    void delete_char_on_cursor();
    void delete_char_before_cursor();
    void handle_newline();
    void write_char(char output_char);
    void write_user_input_new(unsigned int index);
    void execute_command(const std::string& command);

    void print_help();
    void list_workers();
    void exit();
    void start_election(const peg::SemanticValues& values);
    void start_election(const WorkerIdentifier& identifier);
    void stop_ring_or_worker(const peg::SemanticValues& values);
    void stop_ring_or_worker(const WorkerIdentifier& identifier);
    void start_ring_or_worker(const peg::SemanticValues& values);
    void start_ring_or_worker(const WorkerIdentifier& identifier);
    void remove_worker(const peg::SemanticValues& values);
    void remove_worker(const WorkerIdentifier& identifier);

    void clear_line();
    void print_prompt_and_user_input();

  public:
    CommandLine(Presenter* output_writer) {
        set_output_writer(output_writer);
        define_command_parser();
    }

    void set_ring(Ring* ring);

    // Starts the Command Line
    // Returns true when it started successfully
    // Command Line doesn't start and it returns false, when ring isn't set.
    bool start();

    // Stops the Command Line
    void stop();

    // Blocks the current thread until the Command Line is quitted
    void wait();

    // A Loop which constantly reads the console input and acts upon it 
    void operator()();

    void log(spdlog::level::level_enum log_level, const std::string& message) override;

    void ring_created(size_t ring_size) override;
    void ring_starts() override;
    void ring_started() override;
    void ring_stops() override;
    void ring_stopped() override;

    void worker_created(unsigned int worker_id, unsigned int position) override;
    void worker_started(unsigned int position) override;
    void worker_stopped(unsigned int position) override;

    void worker_got_message(unsigned int worker_id, Message* message) override;
    void worker_says(unsigned int worker_id, const std::string& message) override;
    void worker_starts_election(unsigned int worker_id) override;
    void worker_participates_in_election(unsigned int worker_id) override;
    void worker_proposes_itself_in_election(unsigned int worker_id) override;
    void worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) override;
    void worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) override;
    void worker_stops_election_participation(unsigned int worker_id) override;
    void worker_is_elected(unsigned int worker_id) override;
    void worker_resigns_as_leader(unsigned int worker_id) override;

    void election_is_finished(unsigned int leader_id) override;

    void worker_recognizes_dead_neighbour(unsigned int worker_id, unsigned int neighbour_position) override;
    void worker_removes_neighbour(unsigned int worker_id, unsigned int neighbour_position) override;
    void worker_adds_neighbour(unsigned int worker_id, unsigned int neighbour_position) override;

    ~CommandLine();
};
