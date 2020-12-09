#include "worker.h"

using namespace std;


void Worker::assign_message(Message message) {
    message_buffer.assign(move(message));
}

void Worker::set_neighbour(Worker* neighbour) {
    this->neighbour = neighbour;
}

void Worker::operator()() {

}
