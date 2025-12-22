#ifndef DATACALCULATOR_H
#define DATACALCULATOR_H

#include <cstdint>
#include <vector>
#include <string>

//! \brief Класс для вычисления суммы квадратов векторов
//! \details Обрабатывает данные от клиентов, вычисляет сумму квадратов с проверкой переполнения
//! \author Осетров М.С.
//! \date 2025
//! \copyright ПГУ
class DataCalculator {
private:
    static constexpr uint32_t MAX_REASONABLE_VECTORS = 1000;      //!< Максимальное разумное количество векторов
    static constexpr uint32_t MAX_REASONABLE_VECTOR_SIZE = 1000000; //!< Максимальный разумный размер вектора
    
public:
    //! \brief Прочитать точное количество байт из сокета
    //! \param[in] sock Сокет для чтения
    //! \param[out] buffer Буфер для данных
    //! \param[in] size Количество байт для чтения
    //! \return true если успешно, false при ошибке
    //! \throw std::runtime_error При ошибке чтения
    static bool read_exact(int sock, void* buffer, size_t size);
    
    //! \brief Отправить точное количество байт в сокет
    //! \param[in] sock Сокет для отправки
    //! \param[in] buffer Буфер с данными
    //! \param[in] size Количество байт для отправки
    //! \return true если успешно, false при ошибке
    //! \throw std::runtime_error При ошибке отправки
    static bool send_exact(int sock, const void* buffer, size_t size);
    
    //! \brief Обработать данные от клиента
    //! \param[in] client_sock Сокет клиента
    //! \param[in] logger Логгер для записи событий
    //! \param[in] client_ip IP адрес клиента
    //! \return true если обработка успешна, false при ошибке
    static bool process_client_data(int client_sock, class Logger& logger, const std::string& client_ip);
    
    //! \brief Вычислить сумму квадратов вектора
    //! \param[in] vec Вектор значений
    //! \return Сумма квадратов элементов вектора
    //! \throw std::overflow_error При переполнении вычислений
    static double calculate_sum_of_squares(const std::vector<double>& vec);
    
    //! \brief Обработать переполнение значения
    //! \param[in] value Проверяемое значение
    //! \return Значение с ограничением по диапазону
    //! \throw std::overflow_error При недопустимом значении (inf/nan)
    static double handle_overflow(double value);
};

#endif // DATACALCULATOR_H
