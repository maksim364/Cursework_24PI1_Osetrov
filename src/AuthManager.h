#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <string>
#include <unordered_map>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

class Logger;

class AuthManager {
private:
    std::unordered_map<std::string, std::string> users;
    
public:
    AuthManager() = default;
    bool load_users(const std::string& filename);
    bool user_exists(const std::string& login);
    std::string get_password(const std::string& login);
    std::string generate_salt();
    bool authenticate(const std::string& login, 
                      const std::string& client_hash, 
                      const std::string& salt,
                      Logger& logger,
                      const std::string& client_ip);
    static std::string compute_md5_hash(const std::string& salt, const std::string& password);
    const std::unordered_map<std::string, std::string>& get_users() const { return users; }
};

#endif // AUTHMANAGER_H
