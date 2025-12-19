#include "CommandLineParser.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
namespace po = boost::program_options;

CommandLineParser::CommandLineParser() 
    : port(33333), 
      user_db_file("users.txt"),
      log_file("server.log") {}

bool CommandLineParser::parse(int argc, char* argv[]) {
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
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Ошибка парсинга: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        return false;
    }
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return false;
    }
    
    return true;
}

bool CommandLineParser::validate() const {
    if (!fs::exists(user_db_file)) {
        std::cerr << "ОШИБКА: Файл пользователей не найден: " 
                  << fs::absolute(user_db_file) << std::endl;
        std::cerr << "Текущая директория: " << fs::current_path() << std::endl;
        return false;
    }
    return true;
}
