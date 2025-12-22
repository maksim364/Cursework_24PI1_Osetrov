/*! \mainpage Документация сервера обработки данных
 * 
 * \section intro_sec Введение
 * 
 * Сервер для вычисления суммы квадратов векторов с аутентификацией пользователей.
 * 
 * \section features_sec Основные возможности:
 * - Аутентификация пользователей по MD5 хешам
 * - Прием и обработка векторов данных
 * - Вычисление суммы квадратов с проверкой переполнения
 * - Логирование всех событий
 * - Обработка ошибок через исключения
 * 
 * \section usage_sec Использование:
 * \code
 * ./server --port 33333 --users users.txt --log server.log
 * \endcode
 * 
 * \author Осетров М.С.
 * \date 2025
 * \copyright ПГУ
 */

#include "ServerInterface.h"

//! \brief Точка входа в программу
//! \param[in] argc Количество аргументов командной строки
//! \param[in] argv Массив аргументов командной строки
//! \return Код завершения программы
int main(int argc, char* argv[]) {
    return ServerInterface::run(argc, argv);
}
