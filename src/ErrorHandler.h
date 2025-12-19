#pragma once
#include <string>
#include <exception>
#include <memory>
#include "Logger.h"

class ErrorHandler {
private:
    std::shared_ptr<Logger> logger;
    
public:
    explicit ErrorHandler(std::shared_ptr<Logger> logger);
    void handle_network_error(const std::string& context, int error_code = 0);
    void handle_auth_error(const std::string& client_ip, 
                          const std::string& login, 
                          const std::string& reason);
    void handle_calculation_error(const std::string& client_ip, 
                                 const std::string& operation);
    void handle_io_error(const std::string& filename, 
                        const std::string& operation);
    void handle_exception(const std::exception& e, const std::string& context);
    void handle_critical_error(const std::string& error_message);
    void handle_error(const std::string& error_type, 
                     const std::string& message, 
                     bool is_critical = false);
};
