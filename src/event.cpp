#include "event.h"

#include <stdexcept>
#include <string>

using namespace std;


Event::operator string() {
    switch (type) {
        case EventType::RingStarts:
            return "Ring starts.";
        case EventType::RingStarted:
            return "Ring started.";
        case EventType::RingStops:
            return "Ring stops.";
        case EventType::RingStopped:
            return "Ring stopped.";
        default:
            // logically not possible
            throw invalid_argument(
                "String operator on an Event object " 
                "with an illegal EventType has been called."
            );
    }
}

RingCreated::operator string() {
    return "Ring with a size of " + to_string(size) + " has been created.";
}

WorkerStatusChanged::operator string() {
    switch (type) {
        case EventType::WorkerCreated:
            return "Worker " + to_string(worker_id) + " has been created "
                   "on position " + to_string(worker_position) + ".";
        case EventType::WorkerStarted:
            return "Worker " + to_string(worker_id) + " has been started.";
        case EventType::WorkerStopped:
            return "Worker " + to_string(worker_id) + " has been stopped.";
        default:
            // logically not possible
            throw invalid_argument(
                "String operator on an WorkerStatusChanged object " 
                "with an illegal EventType has been called."
            );
    }
}

GotMessage::operator string() {
    return "Worker " + to_string(worker_id) + " got following message: " 
         + (string)*message;
}

Says::operator string() {
    return "Worker " + to_string(worker_id) + " says: \"" + message + "\"";
}

WorkerEvent::operator string() {
    switch (type) {
        case EventType::ElectionStarted:
            return "Worker " + to_string(worker_id) 
                 + " started election.";
        case EventType::Participates:
            return "Worker " + to_string(worker_id) 
                 + " particpates in election.";
        case EventType::ProposedThemselves:
            return "Worker " + to_string(worker_id) 
                 + " proposed themselves as leader.";
        case EventType::ParticipationStopped:
            return "Worker " + to_string(worker_id) 
                 + " no longer participates in election.";
        case EventType::IsElected:
            return "Worker " + to_string(worker_id) 
                 + " has been elected.";
        case EventType::Resigned:
            return "Worker " + to_string(worker_id) 
                 + " is no longer leader.";
        case EventType::ElectionFinished:
            return "election is finished. Worker " + to_string(worker_id) 
                 + " is the leader.";
        default:
            // logically not possible
            throw invalid_argument(
                "String operator on an WorkerEvent object " 
                "with an illegal EventType has been called."
            );
    }
}

ProposalEvent::operator string() {
    switch (type) {
        case EventType::ProposalForwarded:
            return "Worker " + to_string(worker_id) + " forwards election "
                   "proposal for Worker " + to_string(proposal_id) + ".";
        case EventType::ProposalDiscarded:
            return "Worker " + to_string(worker_id) + " discards election "
                   "proposal for Worker " + to_string(proposal_id) + ".";
        default:
            // logically not possible
            throw invalid_argument(
                "String operator on an ProposalEvent object " 
                "with an illegal EventType has been called."
            );
    }
}

ColleagueEvent::operator string() {
    switch (type) {
        case EventType::DeadNeighbourRecognized:
            return "Worker " + to_string(worker_id) + " recognizes their "
                   "neighbour " + to_string(colleague_id) + " doesn't respond.";
        case EventType::ColleagueRemoved:
            return "Worker " + to_string(worker_id) + " removes colleague "
                 + to_string(colleague_id) + " form their colleague list.";
        case EventType::ColleagueAdded:
            return "Worker " + to_string(worker_id) + " adds new colleague "
                 + to_string(colleague_id) + " to their colleague list.";
        default:
            // logically not possible
            throw invalid_argument(
                "String operator on an ColleagueEvent object " 
                "with an illegal EventType has been called."
            );
    }
}
