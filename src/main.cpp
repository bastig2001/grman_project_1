#include "ring.h"
#include "presenters/console_writer.h"

#include "CLI11.hpp"
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

using namespace std;

void cycle(Ring&, chrono::milliseconds);


int main(int argc, char* argv[]) {
    CLI::App app("Simulate a Ring with Elections using the Chang and Roberts algorithm");

    size_t number_of_workers{};
    unsigned int number_of_elections{0};
    unsigned int after_election_sleeptime{5000};
    unsigned int worker_sleeptime{500};

    app.add_option(
        "number-of-workers", 
        number_of_workers, 
        "Number of Workers in the Ring"
    )->required();
    app.add_option(
        "-n, --number-of-elections",
        number_of_elections,
        "Number of Elections after which to finish\n"
        "Default 0 is infinit"
    );
    app.add_option(
        "--sleep",
        after_election_sleeptime,
        "Sleeptime after each Election in milliseconds\n"
        "Default is a sleeptime of 5 seconds"
    );
    app.add_option(
        "--worker-sleep",
        worker_sleeptime,
        "Sleeptime of each worker after a finishing an operation in milliseconds\n"
        "Default is a sleeptime of 500 milliseconds"
    );

    CLI11_PARSE(app, argc, argv);

    auto logger = spdlog::logger("logger");
    logger.set_pattern("");
    logger.set_pattern(".."); // compile the pattern
    ConsoleWriter console_writer(logger, false);

    Ring ring(number_of_workers, worker_sleeptime, &console_writer);
    ring.start();
    
    chrono::milliseconds sleeptime{after_election_sleeptime};
    if (number_of_elections > 0) {
        for (unsigned int i{0}; i < number_of_elections; i++) {
            cycle(ring, sleeptime);
        }
    }
    else {
        while (true) {
            cycle(ring, sleeptime);
        }
    }

    ring.stop();
}

void cycle(Ring& ring, chrono::milliseconds sleeptime) {
    ring.start_election();
    this_thread::sleep_for(sleeptime);
}
