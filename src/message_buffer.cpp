#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::assign(Message* message) {
    unique_lock<mutex> lck{mtx};
    message_assignable.wait(lck, [this](){ return !message_assigned; });

    this->message = message;
    message_assigned = true;
    message_takable.notify_one();
}

Message* MessageBuffer::take() {
    unique_lock<mutex> lck{mtx};
    message_takable.wait(lck, [this](){ return message_assigned; });

    message_assigned = false;
    message_assignable.notify_one();
    return message;
}

bool MessageBuffer::is_empty() {
    return !message_assigned;
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"
#include <chrono>

TEST_CASE(
    "Message Buffer controls and secures access to its contained Message", 
    "[message_buffer][messages]"
) {
    MessageBuffer buffer;
    
    SECTION("messages can't be assigned twice without them beeing taken first") {
        buffer.assign(new NoMessage());
        bool message_taken{false};

        thread t{[&](){
            this_thread::sleep_for(chrono::milliseconds(100));
            message_taken = true;
            delete buffer.take();
        }};

        buffer.assign(new NoMessage());

        CHECK(message_taken);

        t.join();
        buffer.take();
    }

    SECTION("messages can't be taken before they haven't been assigned first") {
        bool message_assigned{false};

        thread t{[&](){
            this_thread::sleep_for(chrono::milliseconds(100));
            message_assigned = true;
            buffer.assign(new NoMessage());
        }};

        delete buffer.take();

        CHECK(message_assigned);

        t.join();
    }

    SECTION("messages don't change; assigned message == taken message") {
        string log_msg_content{ 
            GENERATE(
                "buffer test message",
                "Hello, World!"
            )
        };

        buffer.assign(new LogMessage(log_msg_content));

        auto taken_message{buffer.take()};

        REQUIRE(taken_message->type == MessageType::LogMessage);
        CHECK(taken_message->cast_to<LogMessage>()->content == log_msg_content);

        delete taken_message;
    }

    SECTION("is_empty holds true when the buffer has no message assigned") {
        REQUIRE(buffer.is_empty());

        buffer.assign(new NoMessage());
        CHECK_FALSE(buffer.is_empty());

        delete buffer.take();
        CHECK(buffer.is_empty());
    }
}

#endif
