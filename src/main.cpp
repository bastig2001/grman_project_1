#include "ring.h"
#include "presenters/console_writer.h"

#include "CLI11.hpp"
#include <memory>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thread>
#include <chrono>

using namespace std;

void write_log_start(shared_ptr<spdlog::logger>& logger);
void cycle(Ring&, chrono::milliseconds);


int main(int argc, char* argv[]) {
    size_t number_of_workers{};
    unsigned int number_of_elections{0};
    unsigned int after_election_sleeptime{5000};
    unsigned int worker_sleeptime{500};
    bool logging_enabled{false};
    string log_file_name{""};
    spdlog::level::level_enum logging_level{spdlog::level::off};

    CLI::App app("Simulate a Ring with Elections using the Chang and Roberts algorithm");

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
            "  Default logging level is INFO\n"
            "  Default logging output is the console"
    );
    app.add_option(
        "--log-file",
        log_file_name,
        "Sets the file as log output and enables logging"
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
            "  5 ... CRITICAL"
    );
    
    CLI11_PARSE(app, argc, argv);

    if (logging_enabled && logging_level == spdlog::level::off) {
        logging_level = spdlog::level::info;
    }

    shared_ptr<spdlog::logger> logger{};
    bool is_file_logger{log_file_name != ""};

    if (is_file_logger) {
        logger = spdlog::basic_logger_mt("logger", log_file_name);
        write_log_start(logger);
        logger->set_pattern("[%T.%e] [%l] %v");
    }
    else {
        logger = spdlog::stdout_logger_mt("logger");
        logger->set_pattern("%v");
    }
    
    logger->set_level(logging_level);

    ConsoleWriter console_writer(move(logger), is_file_logger);

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

void write_log_start(shared_ptr<spdlog::logger>& logger) {
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::info);

    logger->info("");
    logger->info("=========================================================");
    logger->set_pattern("  This is a new Log starting at %Y-%m-%d %T.%e");
    logger->info("");
    logger->set_pattern("%v");
    logger->info("=========================================================");
}

void cycle(Ring& ring, chrono::milliseconds sleeptime) {
    ring.start_election();
    this_thread::sleep_for(sleeptime);
}
