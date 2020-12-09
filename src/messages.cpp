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
        default:
            return "Non-Specified Message";
    }
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"



#endif
