#include "ServerInterface.h"
#include "CommandLineParser.h"
#include "Server.h"
#include <iostream>
#include <csignal>
#include <memory>

namespace {
    std::unique_ptr<Server> server_instance;
    
    void signal_handler(int signal) {
        std::cout << "\nПолучен сигнал " << signal << ", завершаем работу..." << std::endl;
        if (server_instance) {
            server_instance->stop();
        }
        exit(0);
    }
}

int ServerInterface::run(int argc, char* argv[]) {
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
    
    server_instance = std::make_unique<Server>(
        parser.get_port(),
        parser.get_user_db_file(),
        parser.get_log_file()
    );
    
    if (!server_instance->start()) {
        std::cerr << "Не удалось запустить сервер!" << std::endl;
        return 1;
    }
    
    server_instance->run();
    
    return 0;
}
