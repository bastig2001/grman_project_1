#include "config.h"
#include "presenters/console_writer.h"

#include "CLI11.hpp"
#include "toml.hpp"
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <fstream>

using namespace std;

shared_ptr<spdlog::logger> get_and_start_logger(const Config&);
void write_log_start(shared_ptr<spdlog::logger>&);


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

    logger->info("=========================================================");
    logger->set_pattern("  This is a new Log starting at %Y-%m-%d %T.%e");
    logger->info("");
    logger->set_pattern("%v");
    logger->info("=========================================================");
}
