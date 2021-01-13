#pragma once

#include "message.h"

#include <spdlog/common.h>


// The possible Types of Events an Event-Object can represent
enum class EventType {
    RingCreated,
    RingStarts,
    RingStarted,
    RingStops,
    RingStopped,

    WorkerCreated,
    WorkerStarted,
    WorkerStopped,

    GotMessage,
    Says,
    ElectionStarted,
    Participates,
    ProposedThemselves,
    ProposalForwarded,
    ProposalDiscarded,
    ParticipationStopped,
    IsElected,
    Resigned,
    ElectionFinished,

    DeadNeighbourRecognized,
    ColleagueRemoved,
    ColleagueAdded,
};


// An Event, meant to represent Information for showing to the user or logging
// only instantiable through the factory methods
struct Event {
    const EventType type;
    const spdlog::level::level_enum logging_level;

    template<typename T>
    T* cast_to() {
        return static_cast<T*>(this);
    }

    virtual operator std::string();

    virtual ~Event() = default;

    static Event* ring_starts() {
        return new Event(EventType::RingStarts, spdlog::level::debug);
    }

    static Event* ring_started() {
        return new Event(EventType::RingStarted, spdlog::level::info);
    }

    static Event* ring_stops() {
        return new Event(EventType::RingStops, spdlog::level::debug);
    }

    static Event* ring_stopped() {
        return new Event(EventType::RingStopped, spdlog::level::info);
    }

  protected:
    Event(
        EventType type, 
        spdlog::level::level_enum logging_level
    ): type{type},
       logging_level{logging_level}
    {}
};


// An Event for the creation of a Ring
struct RingCreated: Event {
    const size_t size;

    RingCreated(
        size_t size
    ): Event(EventType::RingCreated, spdlog::level::debug), 
       size(size) 
    {}

    operator std::string() override;
};


// An Event for when a Worker gets created, started or stopped
// only instantiable through the factory methods
struct WorkerStatusChanged: Event {
    const unsigned int worker_id;
    const unsigned int worker_position;

    operator std::string() override;

    static Event* worker_created(
        unsigned int worker_id, 
        unsigned int worker_position
    ) {
        return new WorkerStatusChanged(
            EventType::WorkerCreated, 
            spdlog::level::debug,
            worker_id, 
            worker_position
        );
    } 

    static Event* worker_started(
        unsigned int worker_id, 
        unsigned int worker_position
    ) {
        return new WorkerStatusChanged(
            EventType::WorkerStarted, 
            spdlog::level::info,
            worker_id, 
            worker_position
        );
    } 

    static Event* worker_Stopped(
        unsigned int worker_id, 
        unsigned int worker_position
    ) {
        return new WorkerStatusChanged(
            EventType::WorkerStopped, 
            spdlog::level::info,
            worker_id, 
            worker_position
        );
    } 

  protected:
    WorkerStatusChanged(
        EventType type, 
        spdlog::level::level_enum logging_level,
        unsigned int worker_id, 
        unsigned int worker_position
    ): Event(type, logging_level),
       worker_id{worker_id},
       worker_position{worker_position}
    {}
};


// An Event for when a Worker got a Message
struct GotMessage: Event {
    const unsigned int worker_id;
    const Message* message;

    GotMessage(
        unsigned int worker_id, 
        Message* message
    ): Event(EventType::GotMessage, spdlog::level::debug),
       worker_id{worker_id},
       message{message}
    {}

    operator std::string() override;
};


// An Event for when a Worker wants to output something
struct Says: Event {
    const unsigned int worker_id;
    const std::string message;

    Says(
        unsigned int worker_id, 
        std::string message
    ): Event(EventType::Says, spdlog::level::info),
       worker_id{worker_id},
       message{message}
    {}

    operator std::string() override;
};


// An Event for when a Worker does something and only their id is necessary
// only instantiable through the factory methods
struct WorkerEvent: Event {
    const unsigned int worker_id;

    operator std::string() override;

    static Event* election_started(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::ElectionStarted, 
            spdlog::level::info,
            worker_id
        );
    }

    static Event* participates(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::Participates, 
            spdlog::level::debug,
            worker_id
        );
    }

    static Event* proposed_themselves(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::ProposedThemselves, 
            spdlog::level::info,
            worker_id
        );
    }

    static Event* participation_stopped(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::ParticipationStopped, 
            spdlog::level::debug,
            worker_id
        );
    }

    static Event* is_elected(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::IsElected, 
            spdlog::level::info,
            worker_id
        );
    }

    static Event* resigned(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::Resigned, 
            spdlog::level::debug,
            worker_id
        );
    }

    static Event* election_finished(unsigned int worker_id) {
        return new WorkerEvent(
            EventType::ElectionFinished, 
            spdlog::level::info,
            worker_id
        );
    }

  protected:
    WorkerEvent(
        EventType type,
        spdlog::level::level_enum logging_level,
        unsigned int worker_id
    ): Event(type, logging_level),
       worker_id{worker_id}
    {}
};


// An Event for when an election proposal gets forwarded or discarded
// only instantiable through the factory methods
struct ProposalEvent: Event {
    const unsigned int worker_id;
    const unsigned int proposal_id;

    operator std::string() override;

    static Event* proposal_forwarded(
        unsigned int worker_id, 
        unsigned int proposal_id
    ) {
        return new ProposalEvent(
            EventType::ProposalForwarded, 
            spdlog::level::info,
            worker_id, 
            proposal_id
        );
    }

    static Event* proposal_discarded(
        unsigned int worker_id, 
        unsigned int proposal_id
    ) {
        return new ProposalEvent(
            EventType::ProposalDiscarded, 
            spdlog::level::info,
            worker_id, 
            proposal_id
        );
    }

  protected:
    ProposalEvent(
        EventType type,
        spdlog::level::level_enum logging_level,
        unsigned int worker_id,
        unsigned int proposal_id
    ): Event(type, logging_level),
       worker_id{worker_id},
       proposal_id{proposal_id}
    {}
};


// An Event for when Worker changes their colleagues
// only instantiable through the factory methods
struct ColleagueEvent: Event {
    const unsigned int worker_id;
    const unsigned int colleague_id;

    operator std::string() override;

    static Event* dead_neighbour_recognized(
        unsigned int worker_id, 
        unsigned int neighbour_id
    ) {
        return new ColleagueEvent(
            EventType::DeadNeighbourRecognized,
            spdlog::level::info,
            worker_id,
            neighbour_id
        );
    }

    static Event* colleague_removed(
        unsigned int worker_id, 
        unsigned int colleague_id
    ) {
        return new ColleagueEvent(
            EventType::ColleagueRemoved,
            spdlog::level::debug,
            worker_id,
            colleague_id
        );
    }

    static Event* colleague_added(
        unsigned int worker_id, 
        unsigned int colleague_id
    ) {
        return new ColleagueEvent(
            EventType::ColleagueAdded,
            spdlog::level::debug,
            worker_id,
            colleague_id
        );
    }

  protected:
    ColleagueEvent(
        EventType type,
        spdlog::level::level_enum logging_level,
        unsigned int worker_id,
        unsigned int colleague_id
    ): Event(type, logging_level),
       worker_id{worker_id},
       colleague_id{colleague_id}
    {}
};


// A static and non-instantiable class for the factory methods for all events
struct CreateEvent: 
    WorkerStatusChanged, 
    WorkerEvent, 
    ProposalEvent, 
    ColleagueEvent 
{
    CreateEvent() = delete;

    static Event* ring_created(size_t size) {
        return new RingCreated(size);
    }

    static Event* got_message(unsigned int worker_id, Message* message) {
        return new GotMessage(worker_id, message);
    }

    static Event* says(unsigned int worker_id, std::string message) {
        return new Says(worker_id, message);
    }
};
