#pragma once

#include "presenters/presenter.h"

// A Presenter which doesn't do anything
class NoPresenter: public Presenter {
  public:
    void log([[maybe_unused]] spdlog::level::level_enum log_level, [[maybe_unused]] const std::string& message) override {}

    void ring_created([[maybe_unused]] size_t ring_size) override {}
    void ring_starts() override {}
    void ring_started() override {}
    void ring_stops() override {}
    void ring_stopped() override {}

    void worker_created([[maybe_unused]] unsigned int worker_id, [[maybe_unused]] unsigned int position) override {}
    void worker_stopped([[maybe_unused]] unsigned int worker_id, [[maybe_unused]] unsigned int position) override {}

    void worker_got_message([[maybe_unused]] unsigned int worker_id, [[maybe_unused]] Message* message) override {}
    void worker_says([[maybe_unused]] unsigned int worker_id, [[maybe_unused]] const std::string& message) override {}
    void worker_starts_election([[maybe_unused]] unsigned int worker_id) override {}
    void worker_participates_in_election([[maybe_unused]] unsigned int worker_id) override {}
    void worker_proposes_itself_in_election([[maybe_unused]] unsigned int worker_id) override {}
    void worker_forwards_election_proposal([[maybe_unused]] unsigned int worker_id, [[maybe_unused]] unsigned int proposal_id) override {}
    void worker_discards_election_proposal([[maybe_unused]] unsigned int worker_id, [[maybe_unused]] unsigned int proposal_id) override {}
    void worker_stops_election_participation([[maybe_unused]] unsigned int worker_id) override {}
    void worker_is_elected([[maybe_unused]] unsigned int worker_id) override {}
    void worker_resigns_as_leader([[maybe_unused]] unsigned int worker_id) override {}

    void election_is_finished([[maybe_unused]] unsigned int leader_id) override {}
};
