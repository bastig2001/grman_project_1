#include "ring.h"
#include "config.h"
#include "presenters/command_line.h"

#include <thread>
#include <chrono>

using namespace std;

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


void run_ring(const Config& config, Presenter* presenter) {
    Ring ring(config.number_of_workers, config.worker_sleeptime, presenter);

    auto command_line{dynamic_cast<CommandLine*>(presenter)};
    if (command_line) {
        command_line->set_ring(&ring);
        command_line->start();
    }

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

    if (command_line) {
        command_line->stop();
    }

    ring.stop();
}

void cycle(Ring& ring, chrono::milliseconds sleeptime) {
    ring.start_election();
    this_thread::sleep_for(sleeptime);
}
