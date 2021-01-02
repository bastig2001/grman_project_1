#pragma once

#include "presenters/logger.h"

// A Presenter and Logger that prints the most important events to the console
class ConsoleWriter: public Logger {
  public:
    ConsoleWriter(
        spdlog::logger logger, 
        bool is_file_logger // set to true, if the given logger logs to a file
    ): Logger(logger, is_file_logger)
    {}

    ~ConsoleWriter();

    void ring_started() override;
    void ring_stopped() override;

    void worker_says(unsigned int worker_id, const std::string& message) override;
    void worker_starts_election(unsigned int worker_id) override;
    void worker_proposes_itself_in_election(unsigned int worker_id) override;
    void worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) override;
    void worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) override;
    void worker_is_elected(unsigned int worker_id) override;

    void election_is_finished(unsigned int leader_id) override;
};
