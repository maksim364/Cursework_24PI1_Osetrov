#ifndef DATACALCULATOR_H
#define DATACALCULATOR_H

#include <cstdint>
#include <vector>
#include <string>

class Logger;

class DataCalculator {
private:
    static constexpr uint32_t MAX_REASONABLE_VECTORS = 1000;
    static constexpr uint32_t MAX_REASONABLE_VECTOR_SIZE = 1000000;
public:
    static bool read_exact(int sock, void* buffer, size_t size);
    static bool send_exact(int sock, const void* buffer, size_t size);
    
    static bool process_client_data(int client_sock, Logger& logger, const std::string& client_ip);
    
    static double calculate_sum_of_squares(const std::vector<double>& vec);
    static double handle_overflow(double value);
};

#endif // DATACALCULATOR_H
