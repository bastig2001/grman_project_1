#include "concrete_presenter.h"

#include <fmt/color.h>
#include <stdexcept>

using namespace std;
using namespace fmt;


void ConcretePresenter::log(
    spdlog::level::level_enum level, 
    const string& message
) {
    file_logger->log(level, message);
    log_to_console(level, message);
}

void ConcretePresenter::set_ring(Ring *ring) {
    if (ring) {
        this->ring = ring;
    }
    else {
        throw invalid_argument("ConcretePresenter needs a pointer to a Ring.");
    }
}

void ConcretePresenter::set_log_to_console() {
    if (console_logger->level() != spdlog::level::off) {
        log_to_console = [this](
            spdlog::level::level_enum level, 
            const std::string& message
        ) {
            lock_guard<mutex> output_lock{output_mtx};
            pre_output();
            console_logger->log(level, message);
            post_output();
        };
    }
}

ConcretePresenter::~ConcretePresenter() {
    stop_command_line();
}

void ConcretePresenter::show(Event* event) {
    file_logger->log(event->logging_level, (string)*event);

    switch (event->type) {
        case EventType::RingStarted:
            ring_started();
            break;
        case EventType::RingStopped:
            ring_stopped();
            break;
        case EventType::Says:
            says(event->cast_to<Says>());
            break;
        case EventType::ElectionStarted:
            election_started(event->cast_to<WorkerEvent>());
            break;
        case EventType::ProposedThemselves:
            proposed_themselves(event->cast_to<WorkerEvent>());
            break;
        case EventType::ProposalForwarded:
            proposal_forwarded(event->cast_to<ProposalEvent>());
            break;
        case EventType::ProposalDiscarded:
            proposal_discarded(event->cast_to<ProposalEvent>());
            break;
        case EventType::IsElected:
            is_elected(event->cast_to<WorkerEvent>());
            break;
        case EventType::ElectionFinished:
            election_finished(event->cast_to<WorkerEvent>());
            break;
        case EventType::DeadNeighbourRecognized: 
            dead_neighbour_recognized(event->cast_to<ColleagueEvent>());
            break;
        default:
            log_to_console(event->logging_level, (string)*event);
            break;
    }

    delete event;
}

void ConcretePresenter::ring_started() {
    println("The Ring has started.");
}

void ConcretePresenter::ring_stopped() {
    println(fg(color::red), "The Ring was stopped.");
}

void ConcretePresenter::says(Says* event) {
    println(
        "Worker {} says: {}", 
        event->worker_id, 
        format(emphasis::italic, event->message)
    );
}

void ConcretePresenter::election_started(WorkerEvent* event) {
    println(
        "Worker {} {}.", 
        event->worker_id, 
        format(emphasis::bold, "started an election")
    );
}

void ConcretePresenter::proposed_themselves(WorkerEvent* event) {
    println(
        "Worker {} {} themselves as leader.", 
        format(fg(color::green_yellow), "{}", event->worker_id), 
        format(emphasis::underline, "proposed")
    );
}

void ConcretePresenter::is_elected(WorkerEvent* event) {
    println(
        "Worker {} {}.",
        format(fg(color::green), "{}", event->worker_id), 
        format(emphasis::underline | fg(color::green), "has been elected")
    );
}

void ConcretePresenter::election_finished(WorkerEvent* event) {
    println(
        fg(color::gold),
        "The election is finished. {} is the leader.",
        format(
            fg(color::fuchsia) | emphasis::underline | emphasis::bold, 
            "Worker {}", event->worker_id
        )
    );
}

void ConcretePresenter::proposal_forwarded(ProposalEvent* event) {
    println(
        "Worker {} {} the election proposal for Worker {}.", 
        event->worker_id, 
        format(emphasis::underline, "forwarded"), 
        format(fg(color::light_blue), "{}", event->proposal_id)
    );
}

void ConcretePresenter::proposal_discarded(ProposalEvent* event) {
    println(
        "Worker {} {} the election proposal for Worker {}.\n",
        event->worker_id, 
        format(emphasis::underline, "discarded"), 
        format(fg(color::fire_brick), "{}", event->proposal_id)
    );
}

void ConcretePresenter::dead_neighbour_recognized(ColleagueEvent* event) {
    println(
        "Worker {} recognized their neighbour {} is {}.\n",
        event->worker_id, event->colleague_id, format(fg(color::red), "dead")
    );
}
