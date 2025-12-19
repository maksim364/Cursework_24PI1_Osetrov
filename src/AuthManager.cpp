#include "AuthManager.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <unistd.h>

bool AuthManager::load_users(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open user database: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    int count = 0;
    
    while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        
        size_t end = line.find_last_not_of(" \t");
        std::string trimmed = line.substr(start, end - start + 1);
        
        if (trimmed.empty() || trimmed[0] == '#') continue;
        
        size_t colon_pos = trimmed.find(':');
        if (colon_pos != std::string::npos) {
            std::string login = trimmed.substr(0, colon_pos);
            std::string password = trimmed.substr(colon_pos + 1);
            
            // Убираем пробелы
            size_t login_start = login.find_first_not_of(" \t");
            size_t login_end = login.find_last_not_of(" \t");
            if (login_start != std::string::npos) {
                login = login.substr(login_start, login_end - login_start + 1);
            }
            
            size_t pass_start = password.find_first_not_of(" \t");
            size_t pass_end = password.find_last_not_of(" \t");
            if (pass_start != std::string::npos) {
                password = password.substr(pass_start, pass_end - pass_start + 1);
            }
            
            users[login] = password;
            count++;
        }
    }
    
    file.close();
    std::cout << "INFO: Loaded " << count << " users from database" << std::endl;
    return true;
}

bool AuthManager::user_exists(const std::string& login) {
    return users.find(login) != users.end();
}

std::string AuthManager::get_password(const std::string& login) {
    auto it = users.find(login);
    return (it != users.end()) ? it->second : "";
}

std::string AuthManager::generate_salt() {
    CryptoPP::byte salt_bytes[8];
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(salt_bytes, sizeof(salt_bytes));
    
    std::string salt_hex;
    CryptoPP::ArraySource as(salt_bytes, sizeof(salt_bytes), true,
        new CryptoPP::HexEncoder(
            new CryptoPP::StringSink(salt_hex)
        )
    );
    
    while (salt_hex.length() < 16) {
        salt_hex = "0" + salt_hex;
    }
    
    if (salt_hex.length() > 16) {
        salt_hex = salt_hex.substr(0, 16);
    }
    
    return salt_hex;
}

std::string AuthManager::compute_md5_hash(const std::string& salt, const std::string& password) {
    std::string digest;
    std::string data_to_hash = salt + password;
    
    CryptoPP::Weak::MD5 hash;
    CryptoPP::StringSource ss(data_to_hash, true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(digest)
            )
        )
    );
    
    for (char& c : digest) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    
    return digest;
}

bool AuthManager::authenticate(const std::string& login, 
                               const std::string& client_hash, 
                               const std::string& salt,
                               Logger& logger,
                               const std::string& client_ip) {
    
    if (!user_exists(login)) {
        logger.log_auth(client_ip, login, false, "User not found");
        return false;
    }
    
    std::string password = get_password(login);
    std::string expected_hash = compute_md5_hash(salt, password);
    
    bool success = (client_hash == expected_hash);
    
    if (success) {
        logger.log_auth(client_ip, login, true, "Hash verified");
    } else {
        logger.log_auth(client_ip, login, false, "Hash mismatch");
    }
    
    return success;
}
