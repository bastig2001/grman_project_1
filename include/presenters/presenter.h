#pragma once

#include "messages.h"

#include <spdlog/spdlog.h>

class Presenter {
  public:
    virtual void log(spdlog::level::level_enum log_level, const std::string& message) = 0;

    virtual void ring_created(size_t ring_size) = 0;
    virtual void ring_starts() = 0;
    virtual void ring_started() = 0;
    virtual void ring_stops() = 0;
    virtual void ring_stopped() = 0;

    virtual void worker_created(unsigned int worker_id, unsigned int position) = 0;
    virtual void worker_stopped(unsigned int worker_id, unsigned int position) = 0;

    virtual void worker_got_message(unsigned int worker_id, Message* message) = 0;
    virtual void worker_says(unsigned int worker_id, const std::string& message) = 0;
    virtual void worker_starts_election(unsigned int worker_id) = 0;
    virtual void worker_participates_in_election(unsigned int worker_id) = 0;
    virtual void worker_proposes_itself_in_election(unsigned int worker_id) = 0;
    virtual void worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) = 0;
    virtual void worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) = 0;
    virtual void worker_stops_election_participation(unsigned int worker_id) = 0;
    virtual void worker_is_elected(unsigned int worker_id) = 0;
    virtual void worker_resigns_as_leader(unsigned int worker_id) = 0;

    virtual void election_is_finished(unsigned int leader_id) = 0;

    virtual ~Presenter() = default;
};
