#include "Logger.h"
#include <iostream>
#include <ctime>
#include <cstring>

std::string Logger::get_current_time() {
    std::time_t now = std::time(nullptr);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buffer);
}

Logger::Logger(const std::string& filename) {
    log_file.open(filename, std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Cannot open log file: " + filename);
    }
    log("=== Server started ===");
}

Logger::~Logger() {
    if (log_file.is_open()) {
        log("=== Server stopped ===");
        log_file.close();
    }
}

void Logger::log(const std::string& message) {
    std::string entry = "[" + get_current_time() + "] " + message;
    std::cout << entry << std::endl;
    
    if (log_file.is_open()) {
        log_file << entry << std::endl;
        log_file.flush();
    }
}

void Logger::log_auth(const std::string& ip, const std::string& login, 
                      bool success, const std::string& details) {
    std::string status = success ? "SUCCESS" : "FAILED";
    std::string msg = "AUTH [" + ip + "] user '" + login + "' : " + status;
    if (!details.empty()) {
        msg += " (" + details + ")";
    }
    log(msg);
}

void Logger::log_connection(const std::string& ip, bool connected) {
    std::string action = connected ? "connected" : "disconnected";
    log("CLIENT [" + ip + "] " + action);
}

void Logger::log_data(const std::string& ip, const std::string& operation) {
    log("DATA [" + ip + "] " + operation);
}

void Logger::log_error(const std::string& error) {
    log("ERROR: " + error);
}

void Logger::log_debug(const std::string& msg) {
    log("DEBUG: " + msg);
}
