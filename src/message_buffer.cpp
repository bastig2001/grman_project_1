#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::assign_sync(Message* message) {
    lock_guard<mutex> rendezvous_lck{rendezvous_mtx};

    message_is_taken = false;
    assign_async(message);

    mutex wait_mtx; // this mutex is only for blocking the current thread
    unique_lock<mutex> wait_lck{wait_mtx}; 
    // rendezvous_lck must not be unlocked while waiting
    message_taken.wait(wait_lck, [this](){ return message_is_taken; });
}

void MessageBuffer::assign_async(Message* message) {
    unique_lock<mutex> buffer_lck{buffer_mtx};
    message_assignable.wait(buffer_lck, [this](){ return is_empty(); });

    this->message = message;
    message_assigned = true;
    message_takable.notify_one();
}

Message* MessageBuffer::take() {
    unique_lock<mutex> buffer_lck{buffer_mtx};
    message_takable.wait(buffer_lck, [this](){ return message_assigned; });

    message_is_taken = true;
    message_assigned = false;
    message_taken.notify_one();
    message_assignable.notify_one();

    return message;
}

bool MessageBuffer::is_empty() {
    return !message_assigned;
}
