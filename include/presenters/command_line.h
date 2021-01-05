#pragma once

#include "presenters/presenter.h"
#include "ring.h"

// A Presenter that provides a command line to interact with the Ring.
// Outputs are written with the given Presenter.
class CommandLine: public Presenter {
  private:
    Presenter* output_writer;
    Ring* ring;

    void set_output_writer(Presenter* output_writer);

  public:
    CommandLine(Presenter* output_writer, Ring* ring): ring{ring} {
        set_output_writer(output_writer);
    }

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
