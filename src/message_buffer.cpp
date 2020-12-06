#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::push(Message message) {
    unique_lock<mutex> lck{mtx};
    message_assignable.wait(lck, [this](){ return !message_set; });

    this->message = message;
    message_set = true;
    message_takable.notify_one();
}

Message MessageBuffer::pop() {
    unique_lock<mutex> lck{mtx};
    message_takable.wait(lck, [this](){ return message_set; });

    message_set = false;
    message_assignable.notify_one();
    return message;
}


#ifdef UNIT_TEST
#include "catch2/catch.hpp"
#include <chrono>

TEST_CASE(
    "Message Buffer controls and secures access to its contained Message", 
    "[message_buffer][messages]"
) {
    auto buffer = MessageBuffer();
    
    SECTION("messages can't be pushed twice without them beeing popped first") {
        buffer.push(Message());
        bool message_popped{false};

        thread t{[&](){
            this_thread::sleep_for(chrono::milliseconds(100));
            buffer.pop();
            message_popped = true;
        }};

        buffer.push(Message());

        CHECK(message_popped);

        t.join();
        buffer.pop();
    }

    SECTION("messages can't be popped before they haven't been pushed first") {
        bool message_pushed{false};

        thread t{[&](){
            this_thread::sleep_for(chrono::milliseconds(100));
            buffer.push(Message());
            message_pushed = true;
        }};

        buffer.pop();

        CHECK(message_pushed);

        t.join();
    }

    SECTION("messages don't change; pushed message == popped message") {
        buffer.push(Message(MessageType::LogMessage, "buffer test message"));

        auto popped_message = buffer.pop();

        CHECK(popped_message.get_type() == MessageType::LogMessage);
        CHECK(popped_message.get_content() == "buffer test message");
    }
}

#endif
