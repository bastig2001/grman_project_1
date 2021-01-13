#include "concrete_presenter.h"
#include "ring.h"
#include "config.h"
#include "presenter.h"
#include "spdlog/common.h"

#include <thread>
#include <chrono>

using namespace std;

void run(const Config&);
void cycle(Ring&, chrono::milliseconds);


int main(int argc, char* argv[]) {
    Config config{};
    ConfigExit config_exit{configure(argc, argv, config)};

    if (config_exit.supposed_to_exit) {
        return config_exit.exit_code;
    }    

    run(config);
}


void run(const Config& config) {
    ConcretePresenter presenter(
        get_console_logger(config), 
        get_and_start_file_logger(config)
    );

    if (!config.no_config_log) {
        presenter.log(
            spdlog::level::debug, 
            "The configuration looks as follows:\n" + (string)config
        );
    }

    Ring ring(config.number_of_workers, config.worker_sleeptime, &presenter);

    if (config.use_command_line) {
        presenter.set_ring(&ring);
        presenter.start_command_line();
    }

    ring.start();
    
    chrono::milliseconds sleeptime{config.after_election_sleeptime};
    
    if (config.number_of_elections > 0 || config.use_command_line) {
        for (unsigned int i{0}; i < config.number_of_elections; i++) {
            cycle(ring, sleeptime);
        }
    }
    else {
        while (true) {
            cycle(ring, sleeptime);
        }
    }

    presenter.wait_for_exit();

    ring.stop();
}

void cycle(Ring& ring, chrono::milliseconds sleeptime) {
    ring.start_election();
    this_thread::sleep_for(sleeptime);
}
