#include "CommandLineParser.h"
#include <iostream>
#include <fstream>  // Добавляем этот заголовочный файл
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;
namespace po = boost::program_options;

CommandLineParser::CommandLineParser() 
    : port(33333), 
      user_db_file("users.txt"),
      log_file("server.log") {}

bool CommandLineParser::parse(int argc, char* argv[]) {
    try {
        po::options_description desc("Сервер обработки данных");
        desc.add_options()
            ("help,h", "Показать справку")
            ("port,p", po::value<int>(&port)->default_value(33333), "Порт сервера")
            ("users,u", po::value<std::string>(&user_db_file)->default_value("users.txt"), 
                       "Файл базы пользователей")
            ("log,l", po::value<std::string>(&log_file)->default_value("server.log"), 
                     "Файл журнала")
        ;
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return false;
        }
        
        // Валидация значений
        if (port <= 0 || port > 65535) {
            throw std::runtime_error("Invalid port number: " + std::to_string(port));
        }
        
        if (user_db_file.empty()) {
            throw std::runtime_error("User database file cannot be empty");
        }
        
        if (log_file.empty()) {
            throw std::runtime_error("Log file cannot be empty");
        }
        
        return true;
        
    } catch (const po::error& e) {
        std::cerr << "Ошибка парсинга командной строки: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return false;
    }
}

bool CommandLineParser::validate() const {
    try {
        if (!fs::exists(user_db_file)) {
            throw std::runtime_error("Файл пользователей не найден: " + 
                                    fs::absolute(user_db_file).string());
        }
        
        // Проверяем, можем ли мы создать/открыть лог-файл
        std::ofstream test_log(log_file, std::ios::app);
        if (!test_log) {
            throw std::runtime_error("Не могу открыть лог-файл для записи: " + 
                                    fs::absolute(log_file).string());
        }
        test_log.close();
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ОШИБКА валидации: " << e.what() << std::endl;
        std::cerr << "Текущая директория: " << fs::current_path() << std::endl;
        return false;
    }
}
