#pragma once

#include "concrete_presenter.h"

#include <spdlog/spdlog.h>


// All configurable values and values resulting from the configuration
struct Config {
    size_t number_of_workers{0};
    std::string config_file{""};
    unsigned int number_of_elections{0};
    unsigned int after_election_sleeptime{5000};
    unsigned int worker_sleeptime{500};
    bool log_to_console{false};
    std::string log_file_name{""};
    bool log_date{false};
    spdlog::level::level_enum general_logging_level{spdlog::level::info};
    spdlog::level::level_enum console_logging_level{spdlog::level::off};
    spdlog::level::level_enum file_logging_level{spdlog::level::off};
    bool no_config_log{false};
    bool use_command_line{false};

    operator std::string() const;
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

// gets the configuration from the user
ConfigExit configure(int argc, char* argv[], Config& config);

// returns a file logger with a start message in the log
// or a turned off console logger 
//      when there is no log_file specified in the config
std::shared_ptr<spdlog::logger> get_and_start_file_logger(const Config& config);
std::shared_ptr<spdlog::logger> get_console_logger(const Config& config);
