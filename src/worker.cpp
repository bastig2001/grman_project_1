#include "worker.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

using namespace std;


void Worker::assign_message(Message* message) {
    message_buffer.assign(message);
}

void Worker::set_neighbour(Worker* neighbour) {
    this->neighbour = neighbour;
}

void Worker::operator()() {
    current_neighbour = neighbour;
    if (current_neighbour) {

        ContinueOperation continue_operation{true};
        while (continue_operation) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue_operation = act_upon_message(message_buffer.take());
        }
    }
}

ContinueOperation Worker::act_upon_message(Message* message) {
    ContinueOperation continue_operation{true};

    spdlog::info("Worker {} got following message: {}.", id, (string)*message);

    switch (message->type) {
        case MessageType::LogMessage:
            spdlog::info("Worker {} says:\n{}", id, message->cast_to<LogMessage>()->content);
            break;
        case MessageType::Stop:
            continue_operation = false;
            break;
        case MessageType::NoMessage:
            break;
    }

    delete message;

    return continue_operation;
}
