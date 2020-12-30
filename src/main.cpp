#include "ring.h"

#include "CLI11.hpp"
#include <thread>
#include <chrono>

using namespace std;

void cycle(Ring&, chrono::milliseconds);


int main(int argc, char* argv[]) {
    CLI::App app("Simulate a Ring with Elections using the Chang and Roberts algorithm");

    size_t number_of_workers{};
    unsigned int number_of_elections{0};
    chrono::milliseconds after_election_sleeptime{5000};

    app.add_option(
        "number-of-workers", 
        number_of_workers, 
        "Number of Workers in the Ring"
    )->required();
    app.add_option(
        "-n, --number-of-elections",
        number_of_elections,
        "Number of Elections after which to finish\n"
        "Default is infinit."
    );
    app.add_option(
        "--sleep",
        after_election_sleeptime,
        "Sleeptime after each Election in milliseconds\n"
        "Default is a sleeptime of 5 seconds."
    );

    CLI11_PARSE(app, argc, argv);

    Ring ring(number_of_workers);
    ring.start();
    
    if (number_of_elections > 0) {
        for (unsigned int i{0}; i < number_of_elections; i++) {
            cycle(ring, after_election_sleeptime);
        }
    }
    else {
        while (true) {
            cycle(ring, after_election_sleeptime);
        }
    }

    ring.stop();
}

void cycle(Ring& ring, chrono::milliseconds sleeptime) {
    ring.start_election();
    this_thread::sleep_for(sleeptime);
}
