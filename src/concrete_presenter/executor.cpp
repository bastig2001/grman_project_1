#include "concrete_presenter.h"

#include <sstream>
#include <string>

using namespace std;
using namespace peg;


void ConcretePresenter::execute(const string& command) {
    command_parser.parse(command.c_str());
}

void ConcretePresenter::define_command_parser() {
    command_parser = (R"(
        Procedure     <- Help / List / Exit / StartElection / Stop / Start
        Help          <- 'help'i / 'h'i
        List          <- 'show'i / 'list'i / 'ls'
        Exit          <- 'quit'i / 'q'i / 'exit'i
        StartElection <- 'start-election'i Pos?
        Stop          <- 'stop'i Pos+
        Start         <- 'start'i Pos+
        Pos           <- UInt
        UInt          <- < [0-9]+ >

        %whitespace   <- [ \t]*
    )");

    command_parser.log = 
        [this](size_t, size_t col, const string& msg) { 
            print_error(col, msg); 
        };
    command_parser["Help"] = 
        [this](const SemanticValues&){ print_help(); };
    command_parser["List"] = 
        [this](const SemanticValues&){ list_workers(); };
    command_parser["Exit"] = 
        [this](const SemanticValues&){ exit(); };
    command_parser["StartElection"] = 
        [this](const SemanticValues& values){ start_election(values); };
    command_parser["Stop"] = 
        [this](const SemanticValues& values){ stop_worker(values); };
    command_parser["Start"] = 
        [this](const SemanticValues& values){ start_worker(values); };
    command_parser["Pos"] = 
        [](const SemanticValues& values){
            return any_cast<unsigned int>(values[0]);
        };
    command_parser["UInt"] = 
        [](const SemanticValues& values){
            return values.token_to_number<unsigned int>();
        };
}

void ConcretePresenter::print_help() {
    println(
        "Following Commands are available:\n"
        "  h, help               outputs this help message\n"
        "  ls, list, show        lists all Workers in the Ring\n"
        "  q, quit, exit         exits the program\n"
        "  start-election [POS]  starts an election with the Worker at the given position or at position 0\n"
        "  stop POS ...          stops the Workers at the given positions\n"
        "  start POS ...         starts the Workers at the given positions\n"
        "\n"
        "  POS  is an unsigned integer\n"
    );
}

void ConcretePresenter::list_workers() {
    auto list{ring->get_worker_list()};
    ostringstream output{};

    output << "Workers:\n";
    for (auto triple : list) {
        unsigned int id{get<0>(triple)};
        unsigned int position{get<1>(triple)};
        string status{get<2>(triple)};

        output << "  Position " << to_string(position) 
               << ": Worker " << to_string(id) 
               << ", Status: " << status << "\n"; 
    }

    println(output.str());
}

void ConcretePresenter::start_election(const SemanticValues& values) {
    if (values.size() == 0) {
        ring->start_election();
    }
    else {
        unsigned int position{any_cast<unsigned int>(values[0])};

        if (ring->start_election_at_position(position)) {
            println("Starting Election...");
        }
        else {
            eprintln("There is no Worker on position ", position, ".");
        }
    }
}

void ConcretePresenter::stop_worker(const SemanticValues&) {

}

void ConcretePresenter::start_worker(const SemanticValues&) {

}

void ConcretePresenter::restart_ring() {
    
}

void ConcretePresenter::print_error(size_t column, const string& err_msg) {
    ostringstream output{};
    string msg{err_msg + " starting here "};
    unsigned long err_position{prompt_length + column - 1};

    for (unsigned long i{0}; i <  err_position; i++) {
        output << " ";
    }
    output << "^\n" << msg;
    for (unsigned long i{msg.size()}; i < err_position; i++) {
        output << "_";
    }
    if (msg.size() < column) {
        output << "|";
    }
    output << "\nRun 'help' for more information.";

    eprintln(output.str());
}
