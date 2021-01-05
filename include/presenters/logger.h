#pragma once

#include "presenters/presenter.h"

// A Presenter that logs everything with the given logger
class Logger: public Presenter {
  private:
    std::shared_ptr<spdlog::logger> logger;
    bool is_file_logger;

  public:
    Logger(
        std::shared_ptr<spdlog::logger> logger, 
        bool is_file_logger
    ): logger{logger}, 
       is_file_logger{is_file_logger} 
    {}

    bool logs_to_file() const {
        return is_file_logger;
    }

    template<typename... Args>
    void log(spdlog::level::level_enum log_level, const Args&... args) {
        logger->log(log_level, args...);
    }

    void log(spdlog::level::level_enum log_level, const std::string& message) override;

    virtual void ring_created(size_t ring_size) override;
    virtual void ring_starts() override;
    virtual void ring_started() override;
    virtual void ring_stops() override;
    virtual void ring_stopped() override;

    virtual void worker_created(unsigned int worker_id, unsigned int position) override;
    virtual void worker_started(unsigned int position) override;
    virtual void worker_stopped(unsigned int position) override;

    virtual void worker_got_message(unsigned int worker_id, Message* message) override;
    virtual void worker_says(unsigned int worker_id, const std::string& message) override;
    virtual void worker_starts_election(unsigned int worker_id) override;
    virtual void worker_participates_in_election(unsigned int worker_id) override;
    virtual void worker_proposes_itself_in_election(unsigned int worker_id) override;
    virtual void worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) override;
    virtual void worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) override;
    virtual void worker_stops_election_participation(unsigned int worker_id) override;
    virtual void worker_is_elected(unsigned int worker_id) override;
    virtual void worker_resigns_as_leader(unsigned int worker_id) override;

    virtual void election_is_finished(unsigned int leader_id) override;

    virtual void worker_recognizes_dead_neighbour(unsigned int worker_id, unsigned int neighbour_position) override;
    virtual void worker_removes_neighbour(unsigned int worker_id, unsigned int neighbour_position) override;
    virtual void worker_adds_neighbour(unsigned int worker_id, unsigned int neighbour_position) override;

    virtual ~Logger();
};
