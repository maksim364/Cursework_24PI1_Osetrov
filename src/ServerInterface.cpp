/*! \file ServerInterface.cpp
 *  \brief Реализация класса ServerInterface
 *  \author Осетров М.С.
 *  \date 2025
 *  \copyright ПГУ
 */

#include "ServerInterface.h"
#include "CommandLineParser.h"
#include "Server.h"
#include "ErrorHandler.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <stdexcept>

namespace {
    std::unique_ptr<Server> server_instance;
    std::shared_ptr<Logger> global_logger;
    std::shared_ptr<ErrorHandler> global_error_handler;
    volatile std::sig_atomic_t signal_received = 0;
    
    void signal_handler(int signal) {
        signal_received = signal;
        std::cout << "\nПолучен сигнал " << signal << ", завершаем работу..." << std::endl;
        
        if (global_logger) {
            global_logger->log("Received signal " + std::to_string(signal) + ", shutting down...");
        }
        
        if (server_instance) {
            server_instance->stop();
        }
    }
}

int ServerInterface::run(int argc, char* argv[]) {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "    Курсовой сервер v1.0" << std::endl;
        std::cout << "    Сумма квадратов векторов" << std::endl;
        std::cout << "========================================" << std::endl;
        
        CommandLineParser parser;
        if (!parser.parse(argc, argv)) {
            return 0; 
        }
        
        if (!parser.validate()) {
            return 1; 
        }
        
        std::cout << "Порт: " << parser.get_port() << std::endl;
        std::cout << "Файл пользователей: " << parser.get_user_db_file() << std::endl;
        std::cout << "Файл лога: " << parser.get_log_file() << std::endl;
        std::cout << "========================================" << std::endl;
        
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        std::signal(SIGPIPE, SIG_IGN); 
        
        try {
            global_logger = std::make_shared<Logger>(parser.get_log_file());
            global_error_handler = std::make_shared<ErrorHandler>(global_logger);
        } catch (const std::exception& e) {
            std::cerr << "FATAL: Cannot initialize logging: " << e.what() << std::endl;
            return 1;
        }
        
        try {
            server_instance = std::make_unique<Server>(
                parser.get_port(),
                parser.get_user_db_file(),
                parser.get_log_file()
            );
        } catch (const std::exception& e) {
            global_error_handler->handle_critical_error("Failed to create server: " + 
                                                       std::string(e.what()));
            return 1;
        }
        
        if (!server_instance->start()) {
            std::cerr << "ERROR: Failed to start server!" << std::endl;
            return 1;
        }
        
        try {
            server_instance->run();
        } catch (const std::exception& e) {
            if (!signal_received) { 
                global_error_handler->handle_critical_error("Server runtime error: " + 
                                                           std::string(e.what()));
            }
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "UNHANDLED EXCEPTION in main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "UNKNOWN EXCEPTION in main" << std::endl;
        return 1;
    }
}
