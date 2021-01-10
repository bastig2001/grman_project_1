#include "message_buffer.h"

#include <thread>
#include <chrono>

using namespace std;


bool MessageBuffer::assign_and_wait(Message* message, unsigned int waittime) {
    // only one thread at a time is allowed to be in this method
    lock_guard<mutex> assign_and_wait_lck{assign_and_wait_mtx};

    unique_lock<mutex> rendezvous_lck{rendezvous_mtx}; 
    message_is_taken = false;
    rendezvous_lck.unlock();

    // rendezvous_lck beeing locked during assignment could cause a deadlock 
    //      in combination with another assign call and take
    assign(message);

    rendezvous_lck.lock();
    return message_taken.wait_for(
        rendezvous_lck, chrono::milliseconds(waittime), 
        [this](){ return message_is_taken; }
    );
}

void MessageBuffer::assign(Message* message) {
    unique_lock<mutex> buffer_lck{buffer_mtx};
    message_assignable.wait(buffer_lck, [this](){ return is_empty(); });

    this->message = message;
    message_assigned = true;
    message_takable.notify_one();
}

Message* MessageBuffer::take() {
    unique_lock<mutex> buffer_lck{buffer_mtx};
    message_takable.wait(buffer_lck, [this](){ return message_assigned; });

    message_assigned = false;
    message_assignable.notify_one();

    lock_guard<mutex> rendezvous_lck{rendezvous_mtx};
    message_is_taken = true;
    message_taken.notify_one();

    return message;
}

bool MessageBuffer::is_empty() {
    return !message_assigned;
}
