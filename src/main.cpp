#include "ring.h"

#include "CLI11.hpp"
#include <thread>
#include <chrono>

using namespace std;


int main(int argc, char* argv[]) {
    CLI::App app("Simulate a Ring with Elections using the Chang and Roberts algorithm");

    size_t number_of_workers{};

    app.add_option(
        "number-of-workers", 
        number_of_workers, 
        "Number of Workers in the Ring"
    )->required();

    CLI11_PARSE(app, argc, argv);

    Ring ring(number_of_workers);
    ring.start();
    ring.start_election();
    this_thread::sleep_for(chrono::seconds(5));
    ring.stop();
}
