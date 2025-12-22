/*! \file ErrorHandler.cpp
 *  \brief Реализация класса ErrorHandler
 *  \details Содержит реализацию методов обработки ошибок
 *  \author Осетров М.С.
 *  \date 2025
 *  \copyright ПГУ
 */

#include "ErrorHandler.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstring>

ErrorHandler::ErrorHandler(std::shared_ptr<Logger> logger) 
    : logger(logger) {}

void ErrorHandler::handle_network_error(const std::string& context, int error_code) {
    std::stringstream ss;
    ss << "Network error [" << context << "]";
    
    if (error_code != 0) {
        ss << ": " << strerror(error_code) << " (code: " << error_code << ")";
    } else if (errno != 0) {
        ss << ": " << strerror(errno) << " (errno: " << errno << ")";
    }
    
    std::string message = ss.str();
    logger->log_error(message);
    std::cerr << "ERROR: " << message << std::endl;
}

void ErrorHandler::handle_auth_error(const std::string& client_ip, 
                                    const std::string& login, 
                                    const std::string& reason) {
    std::string message = "Auth failed [" + client_ip + "] user '" + 
                         login + "': " + reason;
    logger->log_error(message);
    logger->log_auth(client_ip, login, false, reason);
}

void ErrorHandler::handle_calculation_error(const std::string& client_ip, 
                                           const std::string& operation) {
    std::string message = "Calculation error [" + client_ip + "]: " + operation;
    logger->log_error(message);
}

void ErrorHandler::handle_io_error(const std::string& filename, 
                                  const std::string& operation) {
    std::string message = "I/O error: " + operation + " file '" + filename + "'";
    logger->log_error(message);
    std::cerr << "ERROR: " << message << std::endl;
}

void ErrorHandler::handle_exception(const std::exception& e, 
                                   const std::string& context) {
    std::string message = "Exception [" + context + "]: " + e.what();
    logger->log_error(message);
    std::cerr << "EXCEPTION: " << message << std::endl;
}

void ErrorHandler::handle_critical_error(const std::string& error_message) {
    std::string message = "CRITICAL: " + error_message;
    logger->log_error(message);
    std::cerr << "FATAL ERROR: " << message << std::endl;
    
    throw std::runtime_error(message);
}

void ErrorHandler::handle_error(const std::string& error_type, 
                               const std::string& message, 
                               bool is_critical) {
    std::string full_message = error_type + ": " + message;
    
    if (is_critical) {
        logger->log_error("CRITICAL - " + full_message);
        std::cerr << "CRITICAL ERROR: " << full_message << std::endl;
        throw std::runtime_error(full_message);
    } else {
        logger->log_error(full_message);
        std::cerr << "ERROR: " << full_message << std::endl;
    }
}
