#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include "Logger.h"          
#include "AuthManager.h"     
#include "DataCalculator.h"  
#include "ErrorHandler.h"    

//! \brief Основной класс сервера
//! \details Управляет подключениями клиентов, аутентификацией и обработкой данных
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class Server {
private:
    int port;                           //!< Порт сервера
    std::string user_db_file;          //!< Файл базы пользователей
    std::string log_file;              //!< Файл журнала
    
    std::shared_ptr<class Logger> logger;      //!< Логгер
    std::shared_ptr<class ErrorHandler> error_handler; //!< Обработчик ошибок
    class AuthManager auth_manager;            //!< Менеджер аутентификации
    int server_socket;                        //!< Сокет сервера
    bool running;                            //!< Флаг работы сервера
    
public:
    //! \brief Получить IP адрес клиента
    //! \param[in] client_sock Сокет клиента
    //! \return IP адрес клиента или "unknown" при ошибке
    std::string get_client_ip(int client_sock);
    
    //! \brief Отправить строку клиенту
    //! \param[in] sock Сокет клиента
    //! \param[in] str Строка для отправки
    //! \return true если успешно, false при ошибке
    bool send_string(int sock, const std::string& str);
    
    //! \brief Принять строку от клиента
    //! \param[in] sock Сокет клиента
    //! \param[out] str Принятая строка
    //! \param[in] max_len Максимальная длина строки
    //! \return true если успешно, false при ошибке
    bool recv_string(int sock, std::string& str, size_t max_len = 1024);
    
    //! \brief Обработать подключение клиента
    //! \param[in] client_sock Сокет клиента
    //! \return true если обработка завершена (успешно или с ошибкой)
    bool handle_client(int client_sock);
    
    //! \brief Конструктор сервера
    //! \param[in] port Порт для прослушивания
    //! \param[in] user_db_file Файл базы пользователей
    //! \param[in] log_file Файл журнала
    //! \throw std::runtime_error При ошибке инициализации
    Server(int port, const std::string& user_db_file, const std::string& log_file);
    
    //! \brief Деструктор сервера
    ~Server();
    
    //! \brief Запустить сервер
    //! \return true если успешно, false при ошибке
    bool start();
    
    //! \brief Остановить сервер
    void stop();
    
    //! \brief Основной цикл работы сервера
    void run();
};

#endif // SERVER_H
