#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::push(Message message) {
    this->message = message;
}

Message MessageBuffer::pop() {
    return Message();
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"

TEST_CASE(
    "Message Buffer controls and secures access to its contained Message", 
    "[message_buffer][messages]"
) {
}

#endif
