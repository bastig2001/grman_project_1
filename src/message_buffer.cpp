#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::assign_sync(Message* message) {
    // only one thread at a time is allowed to synchronously assign
    lock_guard<mutex> assign_sync_lck{assign_sync_mtx};

    unique_lock<mutex> rendezvous_lck{rendezvous_mtx}; 
    message_is_taken = false;
    rendezvous_lck.unlock();

    // rendezvous_lck beeing locked during assignment could cause a deadlock 
    //      in combination with another assign_async call and take
    assign_async(message);

    rendezvous_lck.lock();
    message_taken.wait(rendezvous_lck, [this](){ return message_is_taken; });
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
