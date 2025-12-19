#include "Server.h"
#include <iostream>
#include <cstring>
#include <csignal>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Server::Server(int port, const std::string& user_db_file, const std::string& log_file)
    : port(port), user_db_file(user_db_file), log_file(log_file),
      server_socket(-1), running(false) {
    
    logger = std::make_unique<Logger>(log_file);
}

Server::~Server() {
    stop();
}

std::string Server::get_client_ip(int client_sock) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_sock, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
    
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
    return std::string(ip_str);
}

bool Server::send_string(int sock, const std::string& str) {
    return send(sock, str.c_str(), str.length(), 0) == static_cast<ssize_t>(str.length());
}

bool Server::recv_string(int sock, std::string& str, size_t max_len) {
    std::vector<char> buffer(max_len + 1);
    
    ssize_t received = recv(sock, buffer.data(), max_len, 0);
    if (received <= 0) {
        return false; 
    }
    
    buffer[received] = '\0';
    str = std::string(buffer.data());
    return true;
}

bool Server::handle_client(int client_sock) {
    std::string client_ip = get_client_ip(client_sock);
    logger->log_connection(client_ip, true);
    
    try {
        std::string login;
        if (!recv_string(client_sock, login)) {
            logger->log_error("No login from " + client_ip);
            close(client_sock);
            return false;
        }
        
        std::string salt = auth_manager.generate_salt();
        
        if (!send_string(client_sock, salt)) {
            logger->log_error("Failed to send salt to " + client_ip);
            close(client_sock);
            return false;
        }
        
        std::string client_hash;
        if (!recv_string(client_sock, client_hash)) {
            logger->log_error("No hash from " + client_ip);
            close(client_sock);
            return false;
        }
        
        bool auth_success = auth_manager.authenticate(
            login, client_hash, salt, *logger, client_ip
        );
        
        if (auth_success) {
            if (!send_string(client_sock, "OK")) {
                logger->log_error("Failed to send OK to " + client_ip);
                close(client_sock);
                return false;
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
        logger->log_error(std::string("Exception handling client: ") + e.what());
    } catch (...) {
        logger->log_error("Unknown error handling client");
    }
    
    close(client_sock);
    logger->log_connection(client_ip, false);
    return true;
}
bool Server::start() {
    logger->log("Starting server on port " + std::to_string(port));
    
    if (!auth_manager.load_users(user_db_file)) {
        logger->log_error("Failed to load user database");
        return false;
    }
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        logger->log_error("Failed to create socket");
        return false;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        logger->log_error("Failed to bind socket");
        close(server_socket);
        return false;
    }
    
    if (listen(server_socket, 10) < 0) {
        logger->log_error("Failed to listen");
        close(server_socket);
        return false;
    }
    
    running = true;
    logger->log("Server started successfully");
    return true;
}

void Server::stop() {
    running = false;
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
    logger->log("Server stopped");
}

void Server::run() {
    logger->log("Waiting for connections...");
    
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_sock = accept(server_socket, 
                                reinterpret_cast<struct sockaddr*>(&client_addr), 
                                &client_len);
        
        if (client_sock < 0) {
            if (running) {
                logger->log_error("Accept failed");
            }
            continue;
        }
        
        handle_client(client_sock);
    }
}
