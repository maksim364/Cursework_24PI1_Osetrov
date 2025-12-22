#pragma once
#include <string>
#include <fstream>

//! \brief Класс для логирования событий сервера
//! \details Записывает логи в файл и выводит в консоль
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class Logger {
private:
    std::ofstream log_file;    //!< Файл журнала
    
    //! \brief Получить текущее время в формате строки
    //! \return Строка с текущим временем
    std::string get_current_time();
    
public:
    //! \brief Конструктор логгера
    //! \param[in] filename Имя файла журнала
    //! \throw std::runtime_error При ошибке открытия файла
    explicit Logger(const std::string& filename);
    
    //! \brief Деструктор логгера
    ~Logger();
    
    //! \brief Записать общее сообщение
    //! \param[in] message Сообщение для записи
    void log(const std::string& message);
    
    //! \brief Записать событие аутентификации
    //! \param[in] ip IP адрес клиента
    //! \param[in] login Логин пользователя
    //! \param[in] success Результат аутентификации
    //! \param[in] details Детали аутентификации (по умолчанию пустая строка)
    void log_auth(const std::string& ip, const std::string& login, 
                  bool success, const std::string& details = "");
    
    //! \brief Записать событие подключения/отключения
    //! \param[in] ip IP адрес клиента
    //! \param[in] connected Флаг подключения (true) или отключения (false)
    void log_connection(const std::string& ip, bool connected);
    
    //! \brief Записать событие обработки данных
    //! \param[in] ip IP адрес клиента
    //! \param[in] operation Операция с данными
    void log_data(const std::string& ip, const std::string& operation);
    
    //! \brief Записать сообщение об ошибке
    //! \param[in] error Текст ошибки
    void log_error(const std::string& error);
    
    //! \brief Записать отладочное сообщение
    //! \param[in] msg Текст отладочного сообщения
    void log_debug(const std::string& msg);
};
