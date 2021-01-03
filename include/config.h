#pragma once

#include "presenters/presenter.h"

#include <spdlog/spdlog.h>

// All configurable values and values resulting from the configuration
struct Config {
    size_t number_of_workers{};
    unsigned int number_of_elections{0};
    unsigned int after_election_sleeptime{5000};
    unsigned int worker_sleeptime{500};
    bool logging_enabled{false};
    std::string log_file_name{""};
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
