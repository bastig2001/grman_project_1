#include "config.h"

#include "CLI11.hpp"
#include "toml.hpp"
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <fstream>
#include <sstream>

using namespace std;

int get_file_config(const string&, Config&);
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
            "  Default 0 is infinit, except --command-line is used, in which case "
              "it immediately goes to the command line without starting an election."
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
        "--log-console",
        config.log_to_console,
        "Logs to the console"
    );
    app.add_option(
        "--log-file",
        config.log_file_name,
        "Logs to the given file"
    );
    app.add_flag(
        "--log-date",
        config.log_date,
        "Logs the date additionally to the time when logging to a file"
    );
    app.add_option(
        "--log-level",
        config.general_logging_level,
        "Sets the lowest visible logging level\n"
            "    0 ... TRACE\n"
            "    1 ... DEBUG\n"
            "    2 ... INFO\n"
            "    3 ... WARN\n"
            "    4 ... ERROR\n"
            "    5 ... CRITICAL\n"
            "  Default is INFO."
    );
    app.add_flag(
        "--no-config-log",
        config.no_config_log,
        "Abstain from logging the used config as a DEBUG message"
    );
    app.add_flag(
        "--command-line",
        config.use_command_line,
        "Use a command line to tell when to start an election and manipulate workers"
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

    if (config.log_to_console) {
        config.console_logging_level = config.general_logging_level;
    }

    if (config.log_file_name != "") {
        config.file_logging_level = config.general_logging_level;
    }

    return false; // Default return, program is not supposed to exit immediately
}

int get_file_config(const string& file_name, Config& config) {
    try {
        auto file_config{toml::parse_file(file_name)};

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

        config.log_to_console = 
            file_config["log"]["console"]
            .value_or(config.log_to_console);
        config.log_file_name = 
            file_config["log"]["file"].value_or(config.log_file_name);
        config.log_date = 
            file_config["log"]["include_date"]
            .value_or(config.log_date);
        config.general_logging_level = (spdlog::level::level_enum)
            file_config["log"]["level"]
            .value_or((int)config.general_logging_level);
        config.no_config_log =
            file_config["log"]["no_config_log"]
            .value_or(config.no_config_log);

        config.use_command_line =
            file_config["command_line"]
            .value_or(config.use_command_line);
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

shared_ptr<spdlog::logger> get_and_start_file_logger(const Config& config) {
    if (config.log_file_name == "") {
        auto logger{spdlog::stdout_logger_st("no file")};
        logger->set_level(spdlog::level::off);

        return logger;
    }
    else {
        try {
            auto logger{
                spdlog::basic_logger_mt("file", config.log_file_name)
            };

            write_log_start(logger);

            string date_pattern{
                config.log_date
                ? "%Y-%m-%d "
                : ""
            };
            logger->set_pattern("[" + date_pattern + "%T.%e] [%l] %v");
            logger->set_level(config.file_logging_level);

            return logger;
        } catch (const spdlog::spdlog_ex& err) {
            cerr << "Writing the log failed:\n" << err.what() << endl;
            exit(3);
        }
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

shared_ptr<spdlog::logger> get_console_logger(const Config& config) {
    auto logger{spdlog::stdout_logger_mt("console")};

    logger->set_pattern("[%l] %v");
    logger->set_level(config.console_logging_level);

    return logger;
}

Config::operator string() const {
    ostringstream output{};
    output << boolalpha
           << "Size:                    " << number_of_workers        << "\n"
           << "Config File:             " << config_file              << "\n"
           << "Number of Elections:     " << number_of_elections      << "\n"
           << "Sleeptime:               " << after_election_sleeptime << " ms\n"
           << "Worker Sleeptime:        " << worker_sleeptime         << " ms\n"
           << "Console Logging enabled: " << log_to_console           << "\n"
           << "Log File:                " << log_file_name            << "\n"
           << "Log Dates in File:       " << log_date                 << "\n"
           << "Logging Level:           " << general_logging_level    << "\n"
           << "Log this Config:         " << !no_config_log           << "\n"
           << "Start Command Line:      " << use_command_line         << "\n";
    
    return output.str();
}
