#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <string>
#include <unordered_map>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

// Предварительное объявление
class Logger;

//! \brief Класс для управления аутентификацией пользователей
//! \details Загружает базу пользователей, генерирует соль, проверяет хеши MD5
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class AuthManager {
private:
    std::unordered_map<std::string, std::string> users; //!< База пользователей (логин → пароль)
    
public:
    //! \brief Конструктор по умолчанию
    AuthManager() = default;
    
    //! \brief Загрузить пользователей из файла
    //! \param[in] filename Имя файла с базой пользователей
    //! \return true если успешно
    //! \throw std::runtime_error При ошибке чтения файла или отсутствии пользователей
    bool load_users(const std::string& filename);
    
    //! \brief Проверить существование пользователя
    //! \param[in] login Логин пользователя
    //! \return true если пользователь существует
    bool user_exists(const std::string& login);
    
    //! \brief Получить пароль пользователя
    //! \param[in] login Логин пользователя
    //! \return Пароль пользователя или пустая строка если пользователь не найден
    std::string get_password(const std::string& login);
    
    //! \brief Сгенерировать случайную соль
    //! \return Соль в шестнадцатеричном формате (16 символов)
    //! \throw std::runtime_error При ошибке генерации
    std::string generate_salt();
    
    //! \brief Аутентифицировать пользователя
    //! \param[in] login Логин пользователя
    //! \param[in] client_hash Хеш от клиента
    //! \param[in] salt Соль использованная для хеширования
    //! \param[in] logger Логгер для записи событий
    //! \param[in] client_ip IP адрес клиента
    //! \return true если аутентификация успешна
    bool authenticate(const std::string& login, 
                      const std::string& client_hash, 
                      const std::string& salt,
                      Logger& logger,
                      const std::string& client_ip);
    
    //! \brief Вычислить MD5 хеш
    //! \param[in] salt Соль
    //! \param[in] password Пароль
    //! \return MD5 хеш в верхнем регистре
    static std::string compute_md5_hash(const std::string& salt, const std::string& password);
    
    //! \brief Получить копию базы пользователей
    //! \return Константная ссылка на базу пользователей
    const std::unordered_map<std::string, std::string>& get_users() const { return users; }
};

#endif // AUTHMANAGER_H
