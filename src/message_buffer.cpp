#include "message_buffer.h"

#include <thread>

using namespace std;


void MessageBuffer::push(Message message) {
    this->message = message;
}

Message MessageBuffer::pop() {
    return Message();
}
