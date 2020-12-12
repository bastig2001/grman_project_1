#include "ring.h"

#include <thread>
#include <chrono>

using namespace std;


int main() {
    Ring ring(5);
    ring.start();
    ring.start_election();
    this_thread::sleep_for(chrono::seconds(5));
    ring.stop();
}
