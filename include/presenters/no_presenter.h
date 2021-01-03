#pragma once

#include "presenters/presenter.h"

// A Presenter which doesn't do anything
class NoPresenter: public Presenter {
  public:
    void log(spdlog::level::level_enum, const std::string&) override {}

    void ring_created(size_t) override {}
    void ring_starts() override {}
    void ring_started() override {}
    void ring_stops() override {}
    void ring_stopped() override {}

    void worker_created(unsigned int, unsigned int) override {}
    void worker_stopped(unsigned int, unsigned int) override {}

    void worker_got_message(unsigned int, Message*) override {}
    void worker_says(unsigned int, const std::string&) override {}
    void worker_starts_election(unsigned int) override {}
    void worker_participates_in_election(unsigned int) override {}
    void worker_proposes_itself_in_election(unsigned int) override {}
    void worker_forwards_election_proposal(unsigned int, unsigned int) override {}
    void worker_discards_election_proposal(unsigned int, unsigned int) override {}
    void worker_stops_election_participation(unsigned int) override {}
    void worker_is_elected(unsigned int) override {}
    void worker_resigns_as_leader(unsigned int) override {}

    void election_is_finished(unsigned int) override {}
};
