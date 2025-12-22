#pragma once
#include <string>
#include <exception>
#include <memory>

//! \brief Класс для обработки ошибок сервера
//! \details Обрабатывает различные типы ошибок: сетевые, аутентификации, вычислений, I/O
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class ErrorHandler {
private:
    std::shared_ptr<class Logger> logger;  //!< Логгер для записи ошибок
    
public:
    //! \brief Конструктор обработчика ошибок
    //! \param[in] logger Общий логгер для записи ошибок
    explicit ErrorHandler(std::shared_ptr<Logger> logger);
    
    //! \brief Обработать сетевую ошибку
    //! \param[in] context Контекст ошибки
    //! \param[in] error_code Код ошибки (0 для использования errno)
    void handle_network_error(const std::string& context, int error_code = 0);
    
    //! \brief Обработать ошибку аутентификации
    //! \param[in] client_ip IP клиента
    //! \param[in] login Логин пользователя
    //! \param[in] reason Причина ошибки
    void handle_auth_error(const std::string& client_ip, 
                          const std::string& login, 
                          const std::string& reason);
    
    //! \brief Обработать ошибку вычислений
    //! \param[in] client_ip IP клиента
    //! \param[in] operation Операция, вызвавшая ошибку
    void handle_calculation_error(const std::string& client_ip, 
                                 const std::string& operation);
    
    //! \brief Обработать I/O ошибку
    //! \param[in] filename Имя файла
    //! \param[in] operation Операция (чтение/запись)
    void handle_io_error(const std::string& filename, 
                        const std::string& operation);
    
    //! \brief Обработать исключение
    //! \param[in] e Исключение
    //! \param[in] context Контекст, где произошло исключение
    void handle_exception(const std::exception& e, const std::string& context);
    
    //! \brief Обработать критическую ошибку
    //! \param[in] error_message Сообщение об ошибке
    //! \throw std::runtime_error Всегда выбрасывает исключение
    void handle_critical_error(const std::string& error_message);
    
    //! \brief Общий метод обработки ошибок
    //! \param[in] error_type Тип ошибки
    //! \param[in] message Сообщение об ошибке
    //! \param[in] is_critical Флаг критичности ошибки (по умолчанию false)
    //! \throw std::runtime_error Если is_critical == true
    void handle_error(const std::string& error_type, 
                     const std::string& message, 
                     bool is_critical = false);
};
