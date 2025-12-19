#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include "Logger.h"
#include "AuthManager.h"
#include "DataCalculator.h"

class Server {
private:
    int port;
    std::string user_db_file;
    std::string log_file;
    
    std::unique_ptr<Logger> logger;
    AuthManager auth_manager;
    int server_socket;
    bool running;
    
    std::string get_client_ip(int client_sock);
    bool send_string(int sock, const std::string& str);
    bool recv_string(int sock, std::string& str, size_t max_len = 1024);
    
    bool handle_client(int client_sock);
    
public:
    Server(int port, const std::string& user_db_file, const std::string& log_file);
    ~Server();
    
    bool start();
    void stop();
    void run();
};

#endif // SERVER_H
