#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>
#include <boost/program_options.hpp>

//! \brief Класс для разбора аргументов командной строки
//! \details Парсит и валидирует параметры запуска сервера
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class CommandLineParser {
private:
    int port;                       //!< Порт сервера
    std::string user_db_file;      //!< Файл базы пользователей
    std::string log_file;          //!< Файл журнала
    
public:
    //! \brief Конструктор парсера командной строки
    CommandLineParser();
    
    //! \brief Разобрать аргументы командной строки
    //! \param[in] argc Количество аргументов
    //! \param[in] argv Массив аргументов
    //! \return true если разбор успешен, false при ошибке или выводе справки
    //! \throw boost::program_options::error При ошибке парсинга
    bool parse(int argc, char* argv[]);
    
    //! \brief Проверить валидность параметров
    //! \return true если параметры валидны
    //! \throw std::runtime_error При ошибке валидации
    bool validate() const;
    
    //! \brief Получить порт сервера
    //! \return Номер порта
    int get_port() const { return port; }
    
    //! \brief Получить файл базы пользователей
    //! \return Путь к файлу базы пользователей
    std::string get_user_db_file() const { return user_db_file; }
    
    //! \brief Получить файл журнала
    //! \return Путь к файлу журнала
    std::string get_log_file() const { return log_file; }
};

#endif // COMMANDLINEPARSER_H
