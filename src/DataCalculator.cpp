/*! \file DataCalculator.cpp
 *  \brief Реализация класса DataCalculator
 *  \details Содержит реализацию методов обработки клиентских данных и их передачу
 *  \author Осетров М.С.
 *  \date 2025
 *  \copyright ПГУ
 */

#include "DataCalculator.h"
#include "Logger.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <limits>
#include <vector>
#include <stdexcept>
#include <cmath>  

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

bool DataCalculator::read_exact(int sock, void* buffer, size_t size) {
    try {
        struct timeval timeout;
        timeout.tv_sec = DATA_PROCESSING_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        
        fd_set read_fds;
        char* ptr = static_cast<char*>(buffer);
        size_t total = 0;
        
        while (total < size) {
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);
            
            int select_result = select(sock + 1, &read_fds, nullptr, nullptr, &timeout);
            
            if (select_result == -1) {
                throw std::runtime_error("select() failed: " + std::string(strerror(errno)));
            } else if (select_result == 0) {
                throw std::runtime_error("Data reading timeout (possible type mismatch: client sends int32_t instead of double)");
            }
            
            ssize_t n = recv(sock, ptr + total, size - total, 0);
            if (n <= 0) {
                if (n == 0) {
                    throw std::runtime_error("Connection closed by client during read");
                } else {
                    throw std::runtime_error("recv failed: " + std::string(strerror(errno)));
                }
            }
            total += n;
            
            timeout.tv_sec = DATA_PROCESSING_TIMEOUT_SEC;
            timeout.tv_usec = 0;
        }
        return true;
    } catch (const std::exception& e) {
        throw;
    }
}

bool DataCalculator::send_exact(int sock, const void* buffer, size_t size) {
    try {
        struct timeval timeout;
        timeout.tv_sec = DATA_PROCESSING_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        
        fd_set write_fds;
        const char* ptr = static_cast<const char*>(buffer);
        size_t total = 0;
        
        while (total < size) {
            FD_ZERO(&write_fds);
            FD_SET(sock, &write_fds);
            
            int select_result = select(sock + 1, nullptr, &write_fds, nullptr, &timeout);
            
            if (select_result == -1) {
                throw std::runtime_error("select() failed for send: " + std::string(strerror(errno)));
            } else if (select_result == 0) {
                throw std::runtime_error("Data sending timeout (client not reading)");
            }
            
            ssize_t n = send(sock, ptr + total, size - total, 0);
            if (n <= 0) {
                if (n == 0) {
                    throw std::runtime_error("Connection closed by client during send");
                } else {
                    throw std::runtime_error("send failed: " + std::string(strerror(errno)));
                }
            }
            total += n;
            
            timeout.tv_sec = DATA_PROCESSING_TIMEOUT_SEC;
            timeout.tv_usec = 0;
        }
        return true;
    } catch (const std::exception& e) {
        throw;
    }
}

double DataCalculator::calculate_sum_of_squares(const std::vector<double>& vec) {
    try {
        double sum = 0.0;
        for (double val : vec) {
            if (val != 0.0 && std::abs(val) > std::numeric_limits<double>::max() / std::abs(val)) {
                throw std::overflow_error("Potential overflow in value squaring");
            }
            double square = val * val;
            
            if (sum != 0.0 && std::abs(square) > std::numeric_limits<double>::max() - std::abs(sum)) {
                throw std::overflow_error("Potential overflow in sum accumulation");
            }
            
            sum += square;
        }
        return handle_overflow(sum);
    } catch (const std::exception& e) {
        throw;
    }
}

double DataCalculator::handle_overflow(double value) {
    const double max_value = std::numeric_limits<double>::max();
    const double min_value = -std::numeric_limits<double>::max();
    
    if (std::isinf(value) || std::isnan(value)) {
        throw std::overflow_error("Invalid floating point value detected");
    }
    
    if (value > max_value) return max_value;
    if (value < min_value) return min_value;
    return value;
}


bool DataCalculator::process_client_data(int client_sock, Logger& logger, const std::string& client_ip) {
    try {
        logger.log_data(client_ip, "Processing client data");
        
        uint32_t num_vectors;
        if (!read_exact(client_sock, &num_vectors, sizeof(num_vectors))) {
            throw std::runtime_error("Failed to read number of vectors");
        }
        
        logger.log_debug("Raw num_vectors: " + std::to_string(num_vectors));
        
        if (num_vectors > MAX_REASONABLE_VECTORS) {
            uint32_t swapped = ntohl(num_vectors);
            logger.log_debug("Large vector count detected (" + std::to_string(num_vectors) + 
                            "), swapped would be: " + std::to_string(swapped));
            
            if (swapped <= MAX_REASONABLE_VECTORS && swapped > 0) {
                num_vectors = swapped;
                logger.log_debug("Using swapped value: " + std::to_string(num_vectors));
            } else {
                throw std::runtime_error("Unreasonable vector count: " + std::to_string(num_vectors));
            }
        }
        
        if (num_vectors == 0) {
            throw std::runtime_error("Zero vectors requested");
        }
        
        logger.log_debug("Client " + client_ip + " will send " + std::to_string(num_vectors) + " vectors");
        
        for (uint32_t vector_idx = 0; vector_idx < num_vectors; vector_idx++) {
            logger.log_debug("Processing vector " + std::to_string(vector_idx + 1) + 
                           "/" + std::to_string(num_vectors));
            
            uint32_t vector_size;
            if (!read_exact(client_sock, &vector_size, sizeof(vector_size))) {
                throw std::runtime_error("Failed to read size for vector " + std::to_string(vector_idx));
            }
            
            if (vector_size > MAX_REASONABLE_VECTOR_SIZE) {
                uint32_t swapped_size = ntohl(vector_size);
                if (swapped_size <= MAX_REASONABLE_VECTOR_SIZE) {
                    vector_size = swapped_size;
                    logger.log_debug("Corrected vector size to " + std::to_string(vector_size));
                } else {
                    throw std::runtime_error("Unreasonable vector size: " + std::to_string(vector_size));
                }
            }
            
            logger.log_debug("Vector " + std::to_string(vector_idx) + " has " + 
                            std::to_string(vector_size) + " elements");
            
            if (vector_size == 0) {
                double zero_result = 0.0;
                if (!send_exact(client_sock, &zero_result, sizeof(zero_result))) {
                    throw std::runtime_error("Failed to send result for empty vector");
                }
                logger.log_debug("Vector " + std::to_string(vector_idx) + " (empty) result: 0.0");
                continue;
            }
            
            std::vector<double> vector_data(vector_size);
            size_t total_bytes_to_read = vector_size * sizeof(double);
            
            if (!read_exact(client_sock, vector_data.data(), total_bytes_to_read)) {
                throw std::runtime_error("Failed to read data for vector " + std::to_string(vector_idx) + 
                                       ", expected " + std::to_string(total_bytes_to_read) + " bytes");
            }
            
            if (vector_size > 0) {
                logger.log_debug("First value of vector " + std::to_string(vector_idx) + 
                               ": " + std::to_string(vector_data[0]));
            }
            
            double vector_result = calculate_sum_of_squares(vector_data);
            
            if (!send_exact(client_sock, &vector_result, sizeof(vector_result))) {
                throw std::runtime_error("Failed to send result for vector " + std::to_string(vector_idx));
            }
            
            logger.log_debug("Vector " + std::to_string(vector_idx) + 
                            " result: " + std::to_string(vector_result));
        }
        
        logger.log_data(client_ip, "Successfully processed " + 
                       std::to_string(num_vectors) + " vectors");
        return true;
        
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        
        // Проверяем, это ли наш таймаут по несовместимости типов
        if (error_msg.find("possible type mismatch") != std::string::npos ||
            error_msg.find("client sends int32_t") != std::string::npos) {
            logger.log_error("Type mismatch detected from " + client_ip + 
                            ": client sends not double");
        }
        
        logger.log_error("Error processing data from " + client_ip + ": " + error_msg);
        return false;
    }
}

