#include "config.h"
#include "presenters/console_writer.h"

#include "CLI11.hpp"
#include "toml.hpp"
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <fstream>
#include <sstream>

using namespace std;

int get_file_config(const string&, Config&);
shared_ptr<spdlog::logger> get_and_start_logger(const Config&);
shared_ptr<spdlog::logger> get_and_start_file_logger(const Config&);
shared_ptr<spdlog::logger> get_console_logger();
void write_log_start(shared_ptr<spdlog::logger>&);


ConfigExit configure(int argc, char* argv[], Config& config) {
    CLI::App app("Simulate a Ring with Elections using the Chang and Roberts algorithm");

    app.add_option(
        "size", 
        config.number_of_workers, 
        "Positive Number of Workers in the Ring\n"
            "  This value needs to be provided either by CLI or config file"
    )->check(CLI::PositiveNumber);
    app.add_option(
        "-c, --config",
        config.config_file,
        "Toml config file from which to read\n"
            "  Configurations can be overriden with the CLI\n"
            "  Values which do not adhere to restrictions are replaced with default values"
    )->check(CLI::ExistingFile);
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
        "Sleeptime of each worker after finishing an operation in milliseconds\n"
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
    app.add_flag(
        "--no-config-log",
        config.no_config_log,
        "Abstain from logging the used config as a DEBUG message"
    );

    
    CLI11_PARSE(app, argc, argv);

    if (config.number_of_workers == 0 && config.config_file == "") {
        cerr << "size or --config is required\n"
             << "Run with --help for more information." << endl;
        return (int)CLI::ExitCodes::RequiredError;
    }
    else if (config.config_file != "") {
        int config_exit{get_file_config(config.config_file, config)};

        if (config_exit) {
            return config_exit;
        }

        // parse the CLI arguments a second time to override the file config
        CLI11_PARSE(app, argc, argv);
    }

    config.is_file_logger = config.log_file_name != "";

    if ((config.logging_enabled || config.is_file_logger) 
            && 
         config.logging_level == spdlog::level::off
    ) {
        config.logging_level = spdlog::level::info;
    }

    return false; // Default return, program is not supposed to exit immediately
}

int get_file_config(const string& file_name, Config& config) {
    try {
        auto file_config = toml::parse_file(file_name);

        config.number_of_workers = 
            file_config["ring"]["size"]
            .value_or(config.number_of_workers);
        config.number_of_elections = 
            file_config["ring"]["number_of_elections"]
            .value_or(config.number_of_elections);
        config.after_election_sleeptime = 
            file_config["ring"]["sleeptime"]
            .value_or(config.after_election_sleeptime);
        config.worker_sleeptime = 
            file_config["ring"]["worker"]["sleeptime"]
            .value_or(config.worker_sleeptime);

        config.logging_enabled = 
            file_config["log"]["enabled"]
            .value_or(config.logging_enabled);
        config.log_file_name = 
            file_config["log"]["file"].value_or(config.log_file_name);
        config.log_date = 
            file_config["log"]["include_date"]
            .value_or(config.log_date);
        config.logging_level = (spdlog::level::level_enum)
            file_config["log"]["level"]
            .value_or((int)config.logging_level);
        config.no_config_log =
            file_config["log"]["no_config_log"]
            .value_or(config.no_config_log);
    }
    catch (const toml::parse_error& err) {
        cerr << "Parsing the Toml config file failed:\n" << err << endl;
        return 1;
    }

    if (config.number_of_workers == 0) {
        cerr << "A positive size is required and needs to be set in the config file or as a CLI argument.\n"
             << "Run with --help for more information." << endl;
        return 2;
    }

    return 0; // Default return, no parsing issues
}

Presenter* get_and_start_presenter(const Config& config) {
    return new ConsoleWriter(get_and_start_logger(config), config.is_file_logger);
}

shared_ptr<spdlog::logger> get_and_start_logger(const Config& config) {
    shared_ptr<spdlog::logger> logger{};

    if (config.is_file_logger) {
        logger = get_and_start_file_logger(config); 
    }
    else {
        logger = get_console_logger();
    }
    
    logger->set_level(config.logging_level);

    if (!config.no_config_log) {
        logger->debug("The configuration looks as follows:\n{}", (string)config);
    }

    return logger;
}

shared_ptr<spdlog::logger> get_and_start_file_logger(const Config& config) {
    try {
        auto logger{spdlog::basic_logger_mt("logger", config.log_file_name)};

        write_log_start(logger);

        string date_pattern{
            config.log_date
            ? "%Y-%m-%d "
            : ""
        };
        logger->set_pattern("[" + date_pattern + "%T.%e] [%l] %v");

        return logger;
    } catch (const spdlog::spdlog_ex& err) {
        cerr << "Writing the log failed:\n" << err.what() << endl;
        exit(3);
    }
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

shared_ptr<spdlog::logger> get_console_logger() {
    auto logger{spdlog::stdout_logger_mt("logger")};

    logger->set_pattern("%v");

    return logger;
}

Config::operator string() const {
    ostringstream output{};
    output << boolalpha
           << "Size:                       " << number_of_workers        << "\n"
           << "Config File:                " << config_file              << "\n"
           << "Number of Elections:        " << number_of_elections      << "\n"
           << "Sleeptime:                  " << after_election_sleeptime << " ms\n"
           << "Worker Sleeptime:           " << worker_sleeptime         << " ms\n"
           << "Logging enabled explicitly: " << logging_enabled          << "\n"
           << "Log File:                   " << log_file_name            << "\n"
           << "Log Dates in File:          " << log_date                 << "\n"
           << "Logging Level:              " << logging_level;
    
    return output.str();
}
