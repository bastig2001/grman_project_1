#include "ring.h"
#include "presenters/console_writer.h"

#include "CLI11.hpp"
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_sinks.h>
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
    bool logging_enabled{false};
    spdlog::level::level_enum logging_level{spdlog::level::off};

    app.add_option(
        "number-of-workers", 
        number_of_workers, 
        "Number of Workers in the Ring"
    )->required();
    app.add_option(
        "-n, --number-of-elections",
        number_of_elections,
        "Number of Elections after which to finish\n"
            "  Default 0 is infinit"
    );
    app.add_option(
        "--sleep",
        after_election_sleeptime,
        "Sleeptime after each Election in milliseconds\n"
            "  Default is a sleeptime of 5 seconds"
    );
    app.add_option(
        "--worker-sleep",
        worker_sleeptime,
        "Sleeptime of each worker after a finishing an operation in milliseconds\n"
            "  Default is a sleeptime of 500 milliseconds"
    );
    app.add_flag(
        "--log",
        logging_enabled,
        "Enables logging\n"
            "  Default logging level is INFO"
    );
    app.add_option(
        "--log-level",
        logging_level,
        "Sets the visible logging level and enables logging\n"
            "  0 ... TRACE\n"
            "  1 ... DEBUG\n"
            "  2 ... INFO\n"
            "  3 ... WARN\n"
            "  4 ... ERROR\n"
            "  5 ... CRITICAL\n"
    );
    
    CLI11_PARSE(app, argc, argv);

    if (logging_enabled && logging_level == spdlog::level::off) {
        logging_level = spdlog::level::info;
    }

    auto logger = spdlog::stdout_logger_mt("logger");
    logger->set_pattern("%v");
    logger->set_level(logging_level);

    ConsoleWriter console_writer(move(logger), false);

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
