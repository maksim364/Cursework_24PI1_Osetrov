/*! \file Server.cpp
 *  \brief Реализация класса Server
 *  \author Осетров М.С.
 *  \date 2025
 *  \copyright ПГУ
 */

#include "Server.h"
#include <iostream>
#include <cstring>
#include <csignal>
#include <stdexcept>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Server::Server(int port, const std::string& user_db_file, const std::string& log_file)
    : port(port), user_db_file(user_db_file), log_file(log_file),
      server_socket(-1), running(false) {
    
    try {
        logger = std::make_shared<Logger>(log_file); 
        error_handler = std::make_shared<ErrorHandler>(logger);
        logger->log("Server initialized successfully");
    } catch (const std::exception& e) {
        std::cerr << "FATAL: Failed to initialize server: " << e.what() << std::endl;
        throw;
    }
}

Server::~Server() {
    stop();
}

std::string Server::get_client_ip(int client_sock) {
    try {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getpeername(client_sock, reinterpret_cast<struct sockaddr*>(&addr), &addr_len) < 0) {
            throw std::runtime_error("getpeername failed");
        }
        
        char ip_str[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str))) {
            throw std::runtime_error("inet_ntop failed");
        }
        
        return std::string(ip_str);
    } catch (const std::exception& e) {
        error_handler->handle_exception(e, "get_client_ip");
        return "unknown";
    }
}

bool Server::send_string(int sock, const std::string& str) {
    try {
        ssize_t sent = send(sock, str.c_str(), str.length(), 0);
        if (sent != static_cast<ssize_t>(str.length())) {
            throw std::runtime_error("send failed, sent " + std::to_string(sent) + 
                                   " bytes instead of " + std::to_string(str.length()));
        }
        return true;
    } catch (const std::exception& e) {
        error_handler->handle_exception(e, "send_string");
        return false;
    }
}

bool Server::recv_string(int sock, std::string& str, size_t max_len) {
    try {
        std::vector<char> buffer(max_len + 1);
        
        ssize_t received = recv(sock, buffer.data(), max_len, 0);
        if (received <= 0) {
            if (received == 0) {
                throw std::runtime_error("connection closed by client");
            } else {
                throw std::runtime_error("recv failed with error");
            }
        }
        
        buffer[received] = '\0';
        str = std::string(buffer.data());
        return true;
    } catch (const std::exception& e) {
        error_handler->handle_exception(e, "recv_string");
        return false;
    }
}

bool Server::handle_client(int client_sock) {
    std::string client_ip = "unknown";
    
    try {
        client_ip = get_client_ip(client_sock);
        logger->log_connection(client_ip, true);
        
        std::string login;
        if (!recv_string(client_sock, login)) {
            throw std::runtime_error("Failed to receive login");
        }
        
        if (login.empty()) {
            throw std::runtime_error("Empty login received");
        }
        
        std::string salt = auth_manager.generate_salt();
        
        if (!send_string(client_sock, salt)) {
            throw std::runtime_error("Failed to send salt");
        }
        
        std::string client_hash;
        if (!recv_string(client_sock, client_hash)) {
            throw std::runtime_error("Failed to receive hash");
        }
        
        bool auth_success = auth_manager.authenticate(
            login, client_hash, salt, *logger, client_ip
        );
        
        if (auth_success) {
            if (!send_string(client_sock, "OK")) {
                throw std::runtime_error("Failed to send OK");
            }
            
            if (!DataCalculator::process_client_data(client_sock, *logger, client_ip)) {
                logger->log_error("Data processing failed for " + client_ip);
            }
        } else {
            if (!send_string(client_sock, "ERR")) {
                logger->log_error("Failed to send ERR to " + client_ip);
            }
        }
        
    } catch (const std::exception& e) {
        error_handler->handle_exception(e, "handle_client for " + client_ip);
        
        try {
            if (!send_string(client_sock, "ERR")) {
                logger->log_error("Failed to send error response");
            }
        } catch (...) {
        }
    }
    
    try {
        if (client_sock >= 0) {
            close(client_sock);
        }
        if (client_ip != "unknown") {
            logger->log_connection(client_ip, false);
        }
    } catch (const std::exception& e) {
        error_handler->handle_exception(e, "cleanup after client");
    }
    
    return true;
}

bool Server::start() {
    try {
        logger->log("Starting server on port " + std::to_string(port));
        
        if (!auth_manager.load_users(user_db_file)) {
            throw std::runtime_error("Failed to load user database: " + user_db_file);
        }
        
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("Failed to set socket options");
        }
        
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            throw std::runtime_error("Failed to bind socket to port " + std::to_string(port));
        }
        
        if (listen(server_socket, 10) < 0) {
            throw std::runtime_error("Failed to listen on socket");
        }
        
        running = true;
        logger->log("Server started successfully on port " + std::to_string(port));
        return true;
        
    } catch (const std::exception& e) {
        error_handler->handle_critical_error(e.what());
        return false;
    }
}

void Server::stop() {
    if (!running) {
        return; 
    }
    
    running = false;
    try {
        if (server_socket >= 0) {
            close(server_socket);
            server_socket = -1;
        }

    } catch (const std::exception& e) {
        if (logger) {
            logger->log_error("Error in Server::stop: " + std::string(e.what()));
        }
    }
}

void Server::run() {
    logger->log("Waiting for connections...");
    
    while (running) {
        try {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_sock = accept(server_socket, 
                                    reinterpret_cast<struct sockaddr*>(&client_addr), 
                                    &client_len);
            
            if (client_sock < 0) {
                if (running) {
                    error_handler->handle_network_error("accept", errno);
                }
                continue;
            }
            
            handle_client(client_sock);
            
        } catch (const std::exception& e) {
            error_handler->handle_exception(e, "Server::run loop");
            

            sleep(1);
        }
    }
}
