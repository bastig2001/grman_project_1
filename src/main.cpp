#include "ring.h"
#include "presenters/console_writer.h"

#include "CLI11.hpp"
#include <memory>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thread>
#include <chrono>
#include <fstream>
#include "toml.hpp"

using namespace std;

// All configurable values and values resulting from the configuration
struct Config {
    size_t number_of_workers{};
    unsigned int number_of_elections{0};
    unsigned int after_election_sleeptime{5000};
    unsigned int worker_sleeptime{500};
    bool logging_enabled{false};
    string log_file_name{""};
    bool log_date{false};
    spdlog::level::level_enum logging_level{spdlog::level::off};
    bool is_file_logger{};
};

// Representing if the program should exit after 'configure' because of the CLI
//      and with which exit code
struct ConfigExit {
    int exit_code;
    bool supposed_to_exit;

    ConfigExit(int exit_code): 
        exit_code{exit_code}, supposed_to_exit{true} 
    {}
    ConfigExit(bool supposed_to_exit): 
        exit_code{}, supposed_to_exit{supposed_to_exit} 
    {}
};

ConfigExit configure(int argc, char* argv[], Config&);
Presenter* get_and_start_presenter(const Config&);
shared_ptr<spdlog::logger> get_and_start_logger(const Config&);
void write_log_start(shared_ptr<spdlog::logger>&);

void run_ring(const Config&, Presenter*);
void cycle(Ring&, chrono::milliseconds);


int main(int argc, char* argv[]) {
    Config config{};
    ConfigExit config_exit{configure(argc, argv, config)};

    if (config_exit.supposed_to_exit) {
        return config_exit.exit_code;
    }    

    Presenter* presenter = get_and_start_presenter(config);
    
    run_ring(config, presenter);

    delete presenter;
}

ConfigExit configure(int argc, char* argv[], Config& config) {
    CLI::App app("Simulate a Ring with Elections using the Chang and Roberts algorithm");

    app.add_option(
        "number-of-workers", 
        config.number_of_workers, 
        "Number of Workers in the Ring"
    )->required();
    app.add_option(
        "-n, --number-of-elections",
        config.number_of_elections,
        "Number of Elections after which to finish\n"
            "  Default 0 is infinit"
    );
    app.add_option(
        "--sleep",
        config.after_election_sleeptime,
        "Sleeptime after each Election in milliseconds\n"
            "  Default is a sleeptime of 5 seconds"
    );
    app.add_option(
        "--worker-sleep",
        config.worker_sleeptime,
        "Sleeptime of each worker after a finishing an operation in milliseconds\n"
            "  Default is a sleeptime of 500 milliseconds"
    );
    app.add_flag(
        "--log",
        config.logging_enabled,
        "Enables logging\n"
            "  Default logging level is INFO\n"
            "  Default logging output is the console"
    );
    app.add_option(
        "--log-file",
        config.log_file_name,
        "Sets the file as log output and enables logging"
    );
    app.add_flag(
        "--log-date",
        config.log_date,
        "Logs the date additionally to the time, when logging to a file"
    );
    app.add_option(
        "--log-level",
        config.logging_level,
        "Sets the visible logging level and enables logging\n"
            "  0 ... TRACE\n"
            "  1 ... DEBUG\n"
            "  2 ... INFO\n"
            "  3 ... WARN\n"
            "  4 ... ERROR\n"
            "  5 ... CRITICAL"
    );
    
    CLI11_PARSE(app, argc, argv);

    config.is_file_logger = config.log_file_name != "";

    if ((config.logging_enabled || config.is_file_logger) 
            && 
         config.logging_level == spdlog::level::off
    ) {
        config.logging_level = spdlog::level::info;
    }

    return false; // Default return, program is not supposed to exit immediately
}

Presenter* get_and_start_presenter(const Config& config) {
    return new ConsoleWriter(get_and_start_logger(config), config.is_file_logger);
}

shared_ptr<spdlog::logger> get_and_start_logger(const Config& config) {
    shared_ptr<spdlog::logger> logger{};

    if (config.is_file_logger) {
        logger = spdlog::basic_logger_mt("logger", config.log_file_name);
        write_log_start(logger);

        string date_pattern{
            config.log_date
            ? "%Y-%m-%d "
            : ""
        };
        logger->set_pattern("[" + date_pattern + "%T.%e] [%l] %v");
    }
    else {
        logger = spdlog::stdout_logger_mt("logger");
        logger->set_pattern("%v");
    }
    
    logger->set_level(config.logging_level);

    return logger;
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

void run_ring(const Config& config, Presenter* presenter) {
    Ring ring(config.number_of_workers, config.worker_sleeptime, presenter);

    ring.start();
    
    chrono::milliseconds sleeptime{config.after_election_sleeptime};
    if (config.number_of_elections > 0) {
        for (unsigned int i{0}; i < config.number_of_elections; i++) {
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
