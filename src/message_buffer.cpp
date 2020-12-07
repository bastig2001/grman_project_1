#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::assign(Message message) {
    unique_lock<mutex> lck{mtx};
    message_assignable.wait(lck, [this](){ return !message_assigned; });

    this->message = move(message);
    message_assigned = true;
    message_takable.notify_one();
}

Message MessageBuffer::take() {
    unique_lock<mutex> lck{mtx};
    message_takable.wait(lck, [this](){ return message_assigned; });

    message_assigned = false;
    message_assignable.notify_one();
    return move(message);
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"
#include <chrono>

TEST_CASE(
    "Message Buffer controls and secures access to its contained Message", 
    "[message_buffer][messages]"
) {
    auto buffer = MessageBuffer();
    
    SECTION("messages can't be assigned twice without them beeing taken first") {
        buffer.assign(Message());
        bool message_taken{false};

        thread t{[&](){
            this_thread::sleep_for(chrono::milliseconds(100));
            buffer.take();
            message_taken = true;
        }};

        buffer.assign(Message());

        CHECK(message_taken);

        t.join();
        buffer.take();
    }

    SECTION("messages can't be taken before they haven't been assigned first") {
        bool message_assigned{false};

        thread t{[&](){
            this_thread::sleep_for(chrono::milliseconds(100));
            buffer.assign(Message());
            message_assigned = true;
        }};

        buffer.take();

        CHECK(message_assigned);

        t.join();
    }

    SECTION("messages don't change; assigned message == taken message") {
        buffer.assign(Message(MessageType::LogMessage, "buffer test message"));

        auto taken_message = buffer.take();

        CHECK(taken_message.type() == MessageType::LogMessage);
        CHECK(taken_message.content() == "buffer test message");
    }
}

#endif
