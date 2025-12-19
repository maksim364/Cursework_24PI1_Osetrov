#pragma once
#include <string>
#include <fstream>

class Logger {
private:
    std::ofstream log_file;
    std::string get_current_time();
    
public:
    explicit Logger(const std::string& filename);
    ~Logger();
    
    void log(const std::string& message);
    void log_auth(const std::string& ip, const std::string& login, 
                  bool success, const std::string& details = "");
    void log_connection(const std::string& ip, bool connected);
    void log_data(const std::string& ip, const std::string& operation);
    void log_error(const std::string& error);
    void log_debug(const std::string& msg);
};
