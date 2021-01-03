#include "presenters/logger.h"

using namespace std;


Logger::~Logger() {
    if (is_file_logger) {
        logger->set_pattern("");
        logger->set_level(spdlog::level::info);
        logger->info("");
    }
}

void Logger::log(spdlog::level::level_enum log_level, const string& message) {
    logger->log(log_level, message);
}

void Logger::ring_created(size_t ring_size) {
    logger->debug("The Ring with a size of {} has been created.", ring_size);
}

void Logger::ring_starts() {
    logger->info("The Ring starts.");
}

void Logger::ring_started() {
    logger->info("The Ring started.");
}

void Logger::ring_stops() {
    logger->info("The Ring stops.");
}

void Logger::ring_stopped() {
    logger->info("The Ring stopped.");
}

void Logger::worker_created(unsigned int worker_id, unsigned int position) {
    logger->debug("The Worker with the id {} has been created on position {}.", worker_id, position);
}

void Logger::worker_stopped(unsigned int worker_id, unsigned int position) {
    logger->debug("The Worker with the id {} on position {} has been stopped.", worker_id, position);
}

void Logger::worker_got_message(unsigned int worker_id, Message* message) {
    logger->debug("Worker {} got following message: {}", worker_id, (string)*message);
}

void Logger::worker_says(unsigned int worker_id, const string& message) {
    logger->info("Worker {} says: \"{}\"", worker_id, message);
}

void Logger::worker_starts_election(unsigned int worker_id) {
    logger->info("Worker {} starts an election.", worker_id);
}

void Logger::worker_participates_in_election(unsigned int worker_id) {
    logger->debug("Worker {} participates in election.", worker_id);
}

void Logger::worker_proposes_itself_in_election(unsigned int worker_id) {
    logger->info("Worker {} proposes itself as leader.", worker_id);
}

void Logger::worker_forwards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    logger->info("Worker {} forwards the election proposal for Worker {}.", worker_id, proposal_id);
}

void Logger::worker_discards_election_proposal(unsigned int worker_id, unsigned int proposal_id) {
    logger->info("Worker {} discards the election proposal for Worker {}.", worker_id, proposal_id);
}

void Logger::worker_stops_election_participation(unsigned int worker_id) {
    logger->debug("Worker {} no longer participates in election.", worker_id);
}

void Logger::worker_is_elected(unsigned int worker_id) {
    logger->info("Worker {} has been elected.", worker_id);
}

void Logger::worker_resigns_as_leader(unsigned int worker_id) {
    logger->debug("Worker {} is no longer the leader.", worker_id);
}

void Logger::election_is_finished(unsigned int leader_id) {
    logger->info("The election is finished. Worker {} is the leader.", leader_id);
}
