#include "message_buffer.h"

#include "catch2/catch.hpp"
#include <chrono>
#include <tuple>
#include <thread>

// sleep is needed since multiple threads are used
#define sleep() this_thread::sleep_for(chrono::milliseconds(25))
#define sleep_more(multiplier) this_thread::sleep_for(chrono::milliseconds(25 * multiplier))

using namespace std;


TEST_CASE(
    "Message Buffer controls and secures access to its contained Message", 
    "[message_buffer]"
) {
    MessageBuffer buffer;
    
    SECTION("messages can't be assigned twice without them beeing taken first") {
        buffer.assign_async(new NoMessage());
        bool message_taken{false};

        thread t{[&](){
            sleep();
            message_taken = true;
            delete buffer.take();
        }};

        buffer.assign_async(new NoMessage());

        CHECK(message_taken);

        t.join();
        delete buffer.take();
    }

    SECTION("messages can't be taken before they haven't been assigned first") {
        bool message_assigned{false};

        thread t{[&](){
            sleep();
            message_assigned = true;
            buffer.assign_async(new NoMessage());
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

        buffer.assign_async(new LogMessage(log_msg_content));

        auto taken_message{buffer.take()};

        REQUIRE(taken_message->type == MessageType::LogMessage);
        CHECK(taken_message->cast_to<LogMessage>()->content == log_msg_content);

        delete taken_message;
    }

    SECTION("is_empty holds true when the buffer has no message assigned") {
        REQUIRE(buffer.is_empty());

        buffer.assign_async(new NoMessage());
        CHECK_FALSE(buffer.is_empty());

        delete buffer.take();
        CHECK(buffer.is_empty());
    }

    SECTION("assign_sync waits for the message to be taken") {
        bool message_taken{false};

        thread t{[&](){
            sleep();
            message_taken = true;
            delete buffer.take();
        }};

        buffer.assign_sync(new NoMessage());

        CHECK(message_taken);

        t.join();
    }

    SECTION("multiple assign_sync calls are handled in the calling order") {
        bool first_message_taken{false};
        bool second_message_taken{false};

        thread t1{[&](){
            sleep();
            buffer.assign_sync(new NoMessage);
            first_message_taken = true;
        }};
        thread t2{[&](){
            sleep_more(2);
            buffer.assign_sync(new NoMessage);
            second_message_taken = true;
        }};

        sleep_more(3);

        delete buffer.take();
        sleep();

        CHECK(first_message_taken);
        CHECK_FALSE(second_message_taken);

        delete buffer.take();
        sleep();

        CHECK(second_message_taken);

        t1.join();
        t2.join();
    }

    SECTION("simultaneous asssign_sync and assign_async calls are both handled and both exit") {
        bool first_message_taken{false};
        bool second_message_assigned{false};

        thread t1{[&](){
            sleep();
            buffer.assign_sync(new NoMessage);
            first_message_taken = true;
        }};
        thread t2{[&](){
            sleep_more(2);
            buffer.assign_async(new NoMessage);
            second_message_assigned = true;
        }};

        sleep_more(3);

        delete buffer.take();
        sleep();

        CHECK(first_message_taken);
        CHECK(second_message_assigned);

        delete buffer.take();

        t1.join();
        t2.join();
    }
}
