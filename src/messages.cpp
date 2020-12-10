#include "messages.h"

using namespace std;


Message::operator string() const {
    switch (type) {
        case MessageType::LogMessage:
            return "Log Message";
        case MessageType::NoMessage:
            return "No Message";
        case MessageType::Stop:
            return "Stop";
        case MessageType::StartElection:
            return "Start Election";
        case MessageType::ElectionProposal:
            return "Election Propsal";
        case MessageType::Elected:
            return "Elected Message";
        default:
            return "Non-Specified Message";
    }
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"



#endif
