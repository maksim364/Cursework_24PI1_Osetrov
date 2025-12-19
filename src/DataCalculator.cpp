#include "DataCalculator.h"
#include "Logger.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <limits>
#include <vector>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

bool DataCalculator::read_exact(int sock, void* buffer, size_t size) {
    char* ptr = static_cast<char*>(buffer);
    size_t total = 0;
    
    while (total < size) {
        ssize_t n = recv(sock, ptr + total, size - total, 0);
        if (n <= 0) {
            return false; 
        }
        total += n;
    }
    return true;
}

bool DataCalculator::send_exact(int sock, const void* buffer, size_t size) {
    const char* ptr = static_cast<const char*>(buffer);
    size_t total = 0;
    
    while (total < size) {
        ssize_t n = send(sock, ptr + total, size - total, 0);
        if (n <= 0) {
            return false; 
        }
        total += n;
    }
    return true;
}

double DataCalculator::calculate_sum_of_squares(const std::vector<double>& vec) {
    double sum = 0.0;
    for (double val : vec) {
        sum += val * val;
    }
    return handle_overflow(sum);
}

double DataCalculator::handle_overflow(double value) {
    const double max_value = 1.7976931348623157e308;
    const double min_value = -1.7976931348623157e308;
    
    if (value > max_value) return max_value;
    if (value < min_value) return min_value;
    return value;
}

bool DataCalculator::process_client_data(int client_sock, Logger& logger, const std::string& client_ip) {
    logger.log_data(client_ip, "Processing client data");
    
    uint32_t num_vectors;
    if (!read_exact(client_sock, &num_vectors, sizeof(num_vectors))) {
        logger.log_error("Failed to read number of vectors from " + client_ip);
        return false;
    }
    
    logger.log_debug("Raw num_vectors: " + std::to_string(num_vectors) + 
                    " (bytes: [" + 
                    std::to_string(((uint8_t*)&num_vectors)[0]) + " " +
                    std::to_string(((uint8_t*)&num_vectors)[1]) + " " +
                    std::to_string(((uint8_t*)&num_vectors)[2]) + " " +
                    std::to_string(((uint8_t*)&num_vectors)[3]) + "])");
    
    if (num_vectors > MAX_REASONABLE_VECTORS) {
        uint32_t swapped = ntohl(num_vectors);
        logger.log_debug("Large vector count detected (" + std::to_string(num_vectors) + 
                        "), swapped would be: " + std::to_string(swapped));
        
        if (swapped <= MAX_REASONABLE_VECTORS && swapped > 0) {
            num_vectors = swapped;
            logger.log_debug("Using swapped value: " + std::to_string(num_vectors));
        } else {
            logger.log_error("Unreasonable vector count: " + std::to_string(num_vectors));
            return false;
        }
    }
    
    logger.log_debug("Client " + client_ip + " will send " + std::to_string(num_vectors) + " vectors");
    
    for (uint32_t vector_idx = 0; vector_idx < num_vectors; vector_idx++) {
        logger.log_debug("Processing vector " + std::to_string(vector_idx + 1) + "/" + std::to_string(num_vectors));
        
        uint32_t vector_size;
        if (!read_exact(client_sock, &vector_size, sizeof(vector_size))) {
            logger.log_error("Failed to read size for vector " + std::to_string(vector_idx) + " from " + client_ip);
            return false;
        }
        
        if (vector_size > MAX_REASONABLE_VECTOR_SIZE) {
            uint32_t swapped_size = ntohl(vector_size);
            if (swapped_size <= MAX_REASONABLE_VECTOR_SIZE) {
                vector_size = swapped_size;
                logger.log_debug("Corrected vector size to " + std::to_string(vector_size));
            } else {
                logger.log_error("Unreasonable vector size: " + std::to_string(vector_size));
                return false;
            }
        }
        
        logger.log_debug("Vector " + std::to_string(vector_idx) + " has " + 
                        std::to_string(vector_size) + " elements");
        
        if (vector_size == 0) {
            double zero_result = 0.0;
            if (!send_exact(client_sock, &zero_result, sizeof(zero_result))) {
                logger.log_error("Failed to send result for empty vector " + std::to_string(vector_idx));
                return false;
            }
            logger.log_debug("Vector " + std::to_string(vector_idx) + " (empty) result: 0.0");
            continue;
        }
        
        std::vector<double> vector_data(vector_size);
        size_t total_bytes_to_read = vector_size * sizeof(double);
        
        if (!read_exact(client_sock, vector_data.data(), total_bytes_to_read)) {
            logger.log_error("Failed to read data for vector " + std::to_string(vector_idx) + 
                           " from " + client_ip + ", expected " + 
                           std::to_string(total_bytes_to_read) + " bytes");
            return false;
        }
        
        if (vector_size > 0) {
            logger.log_debug("First value of vector " + std::to_string(vector_idx) + 
                           ": " + std::to_string(vector_data[0]));
        }
        
        double vector_result = calculate_sum_of_squares(vector_data);
        
        if (!send_exact(client_sock, &vector_result, sizeof(vector_result))) {
            logger.log_error("Failed to send result for vector " + std::to_string(vector_idx) + 
                           " to " + client_ip);
            return false;
        }
        
        logger.log_debug("Vector " + std::to_string(vector_idx) + 
                        " result: " + std::to_string(vector_result));
    }
    
    logger.log_data(client_ip, "Successfully processed " + 
                   std::to_string(num_vectors) + " vectors");
    return true;
}
