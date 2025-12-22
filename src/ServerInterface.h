#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

//! \brief Интерфейс для запуска сервера
//! \details Обрабатывает аргументы командной строки и запускает сервер
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class ServerInterface {
public:
    //! \brief Запустить сервер с параметрами командной строки
    //! \param[in] argc Количество аргументов
    //! \param[in] argv Массив аргументов
    //! \return Код завершения (0 - успешно, 1 - ошибка)
    static int run(int argc, char* argv[]);
};

#endif // SERVERINTERFACE_H
