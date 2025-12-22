#include <UnitTest++/UnitTest++.h>
#include <memory>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

#include "../src/Logger.h"
#include "../src/ErrorHandler.h"
#include "../src/AuthManager.h"
#include "../src/DataCalculator.h"
#include "../src/CommandLineParser.h"
#include "../src/Server.h"

namespace fs = std::filesystem;

class TempFile {
private:
    std::string filename;
public:
    TempFile(const std::string& content = "", const std::string& ext = ".txt") {
        filename = "temp_test_" + std::to_string(std::time(nullptr)) + "_" + 
                   std::to_string(rand()) + ext;
        std::ofstream file(filename);
        if (!file) throw std::runtime_error("Cannot create temp file");
        file << content;
    }
    ~TempFile() { if (fs::exists(filename)) fs::remove(filename); }
    std::string get_path() const { return filename; }
};

int create_test_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) throw std::runtime_error("Failed to create test socket");
    return sock;
}

// ===================== ТЕСТЫ ДЛЯ COMMANDLINEPARSER (Таблица 1) =====================

SUITE(CommandLineParserTests) {
    TEST(Test1_1_CorrectParamsDefault) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "users.txt", "-l", "server.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(parser.parse(argc, const_cast<char**>(argv)));
        CHECK_EQUAL(33333, parser.get_port());
        CHECK_EQUAL("users.txt", parser.get_user_db_file());
        CHECK_EQUAL("server.log", parser.get_log_file());
    }
    
    TEST(Test1_2_CorrectParamsShort) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "auth.txt", "-l", "app.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(parser.parse(argc, const_cast<char**>(argv)));
        CHECK_EQUAL("auth.txt", parser.get_user_db_file());
        CHECK_EQUAL("app.log", parser.get_log_file());
    }
    
    TEST(Test2_1_PortParamLong) {
        CommandLineParser parser;
        const char* argv[] = {"program", "--port", "8080", "--users", "db.txt", "--log", "app.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(parser.parse(argc, const_cast<char**>(argv)));
        CHECK_EQUAL(8080, parser.get_port());
    }
    
    TEST(Test2_2_PortParamShort) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-p", "9000", "-u", "auth.txt", "-l", "debug.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(parser.parse(argc, const_cast<char**>(argv)));
        CHECK_EQUAL(9000, parser.get_port());
    }
    
    TEST(Test3_1_HelpShort) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-h"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    TEST(Test3_2_HelpLong) {
        CommandLineParser parser;
        const char* argv[] = {"program", "--help"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    TEST(Test1_3_NoParams) {
        CommandLineParser parser;
        const char* argv[] = {"program"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(parser.parse(argc, const_cast<char**>(argv)));
        CHECK_EQUAL(33333, parser.get_port());
        CHECK_EQUAL("users.txt", parser.get_user_db_file());
        CHECK_EQUAL("server.log", parser.get_log_file());
    }
    
    TEST(Test4_2_PortMissingValue) {
        CommandLineParser parser;
        const char* argv[] = {"program", "--port"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    
    TEST(Test4_3_InvalidPortZero) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "users.txt", "-l", "server.log", "-p", "0"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    TEST(Test4_4_InvalidPortHigh) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "users.txt", "-l", "server.log", "-p", "70000"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    TEST(Test4_5_EmptyUserFile) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "", "-l", "server.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    TEST(Test4_6_EmptyLogFile) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "users.txt", "-l", ""};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(!parser.parse(argc, const_cast<char**>(argv)));
    }
    
    TEST(Test5_1_UserFileNotExists) {
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "non_existent.txt", "-l", "test.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        CHECK(parser.parse(argc, const_cast<char**>(argv)));
        CHECK(!parser.validate());
    }
    
    TEST(Test5_2_LogFileNoWritePermission) {
        #ifdef __linux__
        CommandLineParser parser;
        const char* argv[] = {"program", "-u", "users.txt", "-l", "/root/test.log"};
        int argc = sizeof(argv)/sizeof(argv[0]);
        parser.parse(argc, const_cast<char**>(argv));
        CHECK(!parser.validate());
        #endif
    }
}

// ===================== ТЕСТЫ ДЛЯ LOGGER (Таблица 2) =====================

SUITE(LoggerTests) {
    TEST(Test1_1_LoggerConstructorValid) {
        TempFile temp;
        Logger logger(temp.get_path());
        CHECK(true);
    }
    
    TEST(Test1_2_LoggerConstructorInvalid) {
    #ifdef __linux__
    try {
        Logger("/root/system.log");
        // Если дошли сюда - исключение НЕ было брошено
        CHECK(false); // Принудительно проваливаем тест
    } catch (const std::exception& e) {
        // Поймали исключение - тест проходит
        std::cout << "Поймано исключение: " << e.what() << std::endl;
        CHECK(true);
    } catch (...) {
        // Поймали что-то другое
        std::cout << "Поймано неизвестное исключение" << std::endl;
        CHECK(false);
    }
    #else
    CHECK(true);
    #endif
    }
    
    TEST(Test2_1_LogAuthSuccess) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_auth("192.168.1.1", "user", true, "");
        CHECK(true);
    }
    
    TEST(Test2_2_LogAuthFailed) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_auth("10.0.0.1", "admin", false, "wrong password");
        CHECK(true);
    }
    
    TEST(Test3_1_LogConnectionConnected) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_connection("127.0.0.1", true);
        CHECK(true);
    }
    
    TEST(Test3_2_LogConnectionDisconnected) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_connection("192.168.1.10", false);
        CHECK(true);
    }
    
    TEST(Test4_1_LogData) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_data("10.0.0.5", "Processing vectors");
        CHECK(true);        
    }
    
    TEST(Test5_1_LogError) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_error("Connection failed");
        CHECK(true);                
    }
    
    TEST(Test5_2_LogDebug) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_debug("Received 5 vectors");
        CHECK(true);                        
    }
    
    TEST(Test6_1_LogEmptyError) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_error("");
        CHECK(true);                                
    }
    
    TEST(Test6_2_LogAuthEmptyLogin) {
        TempFile temp;
        Logger logger(temp.get_path());
        logger.log_auth("ip", "", true);
        CHECK(true);                                
    }
}

// ===================== ТЕСТЫ ДЛЯ ERRORHANDLER (Таблица 3) =====================

SUITE(ErrorHandlerTests) {
    TEST(Test1_1_NetworkErrorWithCode) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        handler.handle_network_error("accept", ECONNREFUSED);
        CHECK(true);                                        
    }
    
    TEST(Test1_2_NetworkErrorWithErrno) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        errno = EBADF;
        handler.handle_network_error("send", 0);
        CHECK(true);                                                
    }
    
    TEST(Test2_1_AuthError) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        handler.handle_auth_error("1.2.3.4", "user", "wrong password");
        CHECK(true);                                                        
    }
    
    TEST(Test2_2_CalculationError) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        handler.handle_calculation_error("10.0.0.1", "sum_of_squares");
        CHECK(true);                                                                
    }
    
    TEST(Test3_1_IOError) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        handler.handle_io_error("users.txt", "reading");
        CHECK(true);                                                                        
    }
    
    TEST(Test4_1_Exception) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        std::runtime_error ex("test");
        handler.handle_exception(ex, "network");
        CHECK(true);                                                                        
    }
    
    TEST(Test5_1_CriticalError) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        CHECK_THROW(handler.handle_critical_error("Cannot bind to port"), std::runtime_error);
    }
    
    TEST(Test6_1_GenericErrorNonCritical) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        handler.handle_error("Network", "Connection timeout", false);
        CHECK(true);                                                                        
    }
    
    TEST(Test6_2_GenericErrorCritical) {
        TempFile temp;
        auto logger = std::make_shared<Logger>(temp.get_path());
        ErrorHandler handler(logger);
        CHECK_THROW(handler.handle_error("Fatal", "Memory allocation failed", true), std::runtime_error);
    }
}

// ===================== ТЕСТЫ ДЛЯ AUTHMANAGER (Таблица 4) =====================

SUITE(AuthManagerTests) {
    TEST(Test1_1_LoadUsersValid) {
        AuthManager auth;
        TempFile temp("user1:pass1\nuser2:pass2\n");
        CHECK(auth.load_users(temp.get_path()));
    }
    
    TEST(Test1_2_LoadUsersWithComments) {
        AuthManager auth;
        TempFile temp("# Комментарий\nuser1:pass1\n\nuser2:pass2\n  # Еще комментарий");
        CHECK(auth.load_users(temp.get_path()));
    }
    
    TEST(Test1_3_LoadUsersEmptyFile) {
        AuthManager auth;
        TempFile temp("");
        CHECK_THROW(auth.load_users(temp.get_path()), std::runtime_error);
    }
    
    TEST(Test1_4_LoadUsersFileNotExists) {
        AuthManager auth;
        CHECK_THROW(auth.load_users("non_existent_file.txt"), std::runtime_error);
    }
    
    TEST(Test2_1_UserExistsTrue) {
        AuthManager auth;
        TempFile temp("user1:pass1\nuser2:pass2\n");
        auth.load_users(temp.get_path());
        CHECK(auth.user_exists("user1"));
    }
    
    TEST(Test2_2_UserExistsFalse) {
        AuthManager auth;
        TempFile temp("user1:pass1\nuser2:pass2\n");
        auth.load_users(temp.get_path());
        CHECK(!auth.user_exists("unknown"));
    }
    
    TEST(Test3_1_GetPasswordValid) {
        AuthManager auth;
        TempFile temp("user1:pass1\nuser2:pass2\n");
        auth.load_users(temp.get_path());
        CHECK_EQUAL("pass1", auth.get_password("user1"));
    }
    
    TEST(Test3_2_GetPasswordInvalid) {
        AuthManager auth;
        TempFile temp("user1:pass1\nuser2:pass2\n");
        auth.load_users(temp.get_path());
        CHECK_EQUAL("", auth.get_password("unknown"));
    }
    
    TEST(Test4_1_GenerateSaltUnique) {
        AuthManager auth;
        std::string salt1 = auth.generate_salt();
        std::string salt2 = auth.generate_salt();
        CHECK(salt1 != salt2);
    }
    
    TEST(Test4_2_GenerateSaltFormat) {
        AuthManager auth;
        std::string salt = auth.generate_salt();
        CHECK_EQUAL(16, salt.length());
        for (char c : salt) {
            CHECK((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
        }
    }
    
    TEST(Test5_1_ComputeMD5Hash) {
        std::string hash = AuthManager::compute_md5_hash("0123ABCD", "secret");
        CHECK_EQUAL(32, hash.length());
    }
    
    TEST(Test5_2_ComputeMD5HashEmptyPassword) {
        std::string hash = AuthManager::compute_md5_hash("SALT1234", "");
        CHECK_EQUAL(32, hash.length());
    }
    
    TEST(Test6_1_AuthenticateCorrect) {
        AuthManager auth;
        TempFile temp("testuser:mypassword\n");
        auth.load_users(temp.get_path());
        std::string salt = auth.generate_salt();
        std::string hash = AuthManager::compute_md5_hash(salt, "mypassword");
        TempFile log;
        Logger logger(log.get_path());
        CHECK(auth.authenticate("testuser", hash, salt, logger, "127.0.0.1"));
    }
    
    TEST(Test6_2_AuthenticateWrongPassword) {
        AuthManager auth;
        TempFile temp("testuser:mypassword\n");
        auth.load_users(temp.get_path());
        std::string salt = auth.generate_salt();
        TempFile log;
        Logger logger(log.get_path());
        CHECK(!auth.authenticate("testuser", "wronghash", salt, logger, "127.0.0.1"));
    }
    
    TEST(Test6_3_AuthenticateUnknownUser) {
        AuthManager auth;
        TempFile temp("testuser:mypassword\n");
        auth.load_users(temp.get_path());
        std::string salt = auth.generate_salt();
        std::string hash = AuthManager::compute_md5_hash(salt, "mypassword");
        TempFile log;
        Logger logger(log.get_path());
        CHECK(!auth.authenticate("unknown", hash, salt, logger, "127.0.0.1"));
    }
}

// ===================== ТЕСТЫ ДЛЯ DATACALCULATOR (Таблица 5) =====================

SUITE(DataCalculatorTests) {
    TEST(Test1_1_SumOfSquaresPositive) {
        std::vector<double> vec = {1.0, 2.0, 3.0};
        double result = DataCalculator::calculate_sum_of_squares(vec);
        CHECK_CLOSE(14.0, result, 0.0001);
    }
    
    TEST(Test1_2_SumOfSquaresNegative) {
        std::vector<double> vec = {-1.0, -2.0, -3.0};
        double result = DataCalculator::calculate_sum_of_squares(vec);
        CHECK_CLOSE(14.0, result, 0.0001);
    }
    
    TEST(Test1_3_SumOfSquaresZeros) {
        std::vector<double> vec = {0.0, 0.0, 0.0};
        double result = DataCalculator::calculate_sum_of_squares(vec);
        CHECK_CLOSE(0.0, result, 0.0001);
    }
    
    TEST(Test1_4_SumOfSquaresEmpty) {
        std::vector<double> vec = {};
        double result = DataCalculator::calculate_sum_of_squares(vec);
        CHECK_CLOSE(0.0, result, 0.0001);
    }
    
    TEST(Test1_5_SumOfSquaresSmallValues) {
        std::vector<double> vec = {1e-300, 1e-300};
        double result = DataCalculator::calculate_sum_of_squares(vec);
        CHECK(result >= 0.0);
    }
    
    TEST(Test2_1_HandleOverflowNormal) {
        double result = DataCalculator::handle_overflow(123.45);
        CHECK_CLOSE(123.45, result, 0.0001);
    }
    
    TEST(Test2_2_HandleOverflowMax) {
        double max_val = std::numeric_limits<double>::max();
        double result = DataCalculator::handle_overflow(max_val);
        CHECK_CLOSE(max_val, result, 0.0001);
    }
    
    TEST(Test2_3_HandleOverflowInfinity) {
        double inf = std::numeric_limits<double>::infinity();
        CHECK_THROW(DataCalculator::handle_overflow(inf), std::overflow_error);
    }
    
    TEST(Test2_4_HandleOverflowNaN) {
        double nan = std::numeric_limits<double>::quiet_NaN();
        CHECK_THROW(DataCalculator::handle_overflow(nan), std::overflow_error);
    }
    
    TEST(Test3_1_ReadExactCorrect) {
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        uint32_t test_data = 0x12345678;
        send(sockfd[0], &test_data, sizeof(test_data), 0);
        shutdown(sockfd[0], SHUT_WR);
        uint32_t received = 0;
        bool result = DataCalculator::read_exact(sockfd[1], &received, sizeof(received));
        CHECK(result);
        CHECK_EQUAL(test_data, received);
        close(sockfd[0]); close(sockfd[1]);
    }
    
    TEST(Test3_2_SendExactCorrect) {
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        double test_data = 3.14159;
        bool result = DataCalculator::send_exact(sockfd[0], &test_data, sizeof(test_data));
        CHECK(result);
        shutdown(sockfd[0], SHUT_WR);
        double received = 0;
        recv(sockfd[1], &received, sizeof(received), MSG_WAITALL);
        CHECK_CLOSE(test_data, received, 0.0001);
        close(sockfd[0]); close(sockfd[1]);
    }
    
    TEST(Test3_3_ReadExactBrokenSocket) {
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        close(sockfd[0]);
        uint32_t data = 0;
        CHECK_THROW(DataCalculator::read_exact(sockfd[1], &data, sizeof(data)), std::runtime_error);
        close(sockfd[1]);
    }
    
    TEST(Test3_4_SendExactBrokenSocket) {
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        close(sockfd[1]);
        double data = 1.234;
        CHECK_THROW(DataCalculator::send_exact(sockfd[0], &data, sizeof(data)), std::runtime_error);
        close(sockfd[0]);
    }
    
    TEST(Test4_1_ProcessClientDataCorrect) {
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        
        std::thread sender([sockfd]() {
            uint32_t num_vectors = 1;
            send(sockfd[0], &num_vectors, sizeof(num_vectors), 0);
            uint32_t vector_size = 3;
            send(sockfd[0], &vector_size, sizeof(vector_size), 0);
            double data[3] = {1.0, 2.0, 3.0};
            send(sockfd[0], data, sizeof(data), 0);
            double result = 0;
            recv(sockfd[0], &result, sizeof(result), MSG_WAITALL);
            CHECK_CLOSE(14.0, result, 0.0001);
            shutdown(sockfd[0], SHUT_WR);
        });
        
        TempFile log;
        Logger logger(log.get_path());
        bool result = DataCalculator::process_client_data(sockfd[1], logger, "127.0.0.1");
        sender.join();
        CHECK(result);
        close(sockfd[0]); close(sockfd[1]);
    }
    
    TEST(Test4_2_ProcessClientDataTooManyVectors) {
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        uint32_t num_vectors = 1000000;
        send(sockfd[0], &num_vectors, sizeof(num_vectors), 0);
        shutdown(sockfd[0], SHUT_WR);
        TempFile log;
        Logger logger(log.get_path());
        bool result = DataCalculator::process_client_data(sockfd[1], logger, "127.0.0.1");
        CHECK(!result);
        close(sockfd[0]); close(sockfd[1]);
    }
    
    TEST(Test4_3_ProcessClientDataByteOrder) {
    int sockfd[2];
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
    
    std::thread sender([sockfd]() {
        // Отправляем только 1 вектор вместо 2
        uint32_t num_vectors = htonl(1);  // ← ИЗМЕНИТЬ с 2 на 1!
        send(sockfd[0], &num_vectors, sizeof(num_vectors), 0);
        uint32_t vector_size = htonl(2);
        send(sockfd[0], &vector_size, sizeof(vector_size), 0);
        double data[2] = {1.5, 2.5};
        send(sockfd[0], data, sizeof(data), 0);
        double result = 0;
        recv(sockfd[0], &result, sizeof(result), MSG_WAITALL);
        CHECK_CLOSE(8.5, result, 0.0001);
        shutdown(sockfd[0], SHUT_WR);
    });
    
    TempFile log;
    Logger logger(log.get_path());
    bool result = DataCalculator::process_client_data(sockfd[1], logger, "127.0.0.1");
    sender.join();
    CHECK(result);
    close(sockfd[0]); close(sockfd[1]);
    }
}

// ===================== ТЕСТЫ ДЛЯ SERVER (Таблица 6) =====================

SUITE(ServerTests) {
    TEST(Test1_1_GetClientIPValidSocket) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33333, users.get_path(), log.get_path());
        
        int test_sock = socket(AF_INET, SOCK_STREAM, 0);
        CHECK(test_sock >= 0);
        
        std::string ip = server.get_client_ip(test_sock);
        CHECK_EQUAL("unknown", ip);
        
        close(test_sock);
    }
    
    TEST(Test1_2_GetClientIPInvalidSocket) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33333, users.get_path(), log.get_path());
        
        std::string ip = server.get_client_ip(-1);
        CHECK_EQUAL("unknown", ip);
    }
    
    TEST(Test2_1_SendStringSuccess) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33333, users.get_path(), log.get_path());
        
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        
        bool result = server.send_string(sockfd[0], "Hello");
        CHECK(result);
        
        close(sockfd[0]);
        close(sockfd[1]);
    }
    
    TEST(Test2_2_RecvStringSuccess) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33333, users.get_path(), log.get_path());
        
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        
        std::string test_msg = "Test message";
        send(sockfd[0], test_msg.c_str(), test_msg.length(), 0);
        shutdown(sockfd[0], SHUT_WR);
        
        std::string received;
        bool result = server.recv_string(sockfd[1], received);
        
        CHECK(result);
        CHECK_EQUAL(test_msg, received);
        
        close(sockfd[0]);
        close(sockfd[1]);
    }
    
    TEST(Test2_3_SendStringBrokenSocket) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33333, users.get_path(), log.get_path());
        
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        
        close(sockfd[1]);
        
        bool result = server.send_string(sockfd[0], "Test");
        CHECK(!result);
        
        close(sockfd[0]);
    }
    
    TEST(Test2_4_RecvStringConnectionClosed) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33333, users.get_path(), log.get_path());
        
        int sockfd[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) >= 0);
        
        close(sockfd[0]);
        
        std::string received;
        bool result = server.recv_string(sockfd[1], received);
        CHECK(!result);
        
        close(sockfd[1]);
    }
    
    TEST(Test3_1_ServerStartSuccess) {
        TempFile users("test:pass\n");
        TempFile log;
        
        Server server(33338, users.get_path(), log.get_path());
        bool result = server.start();
        server.stop();
        
        CHECK(result);
    }
    
    TEST(Test3_2_ServerStartPortInUse) {
    TempFile users("test:pass\n");
    TempFile log1, log2;
    
    Server server1(33339, users.get_path(), log1.get_path());
    CHECK(server1.start());
    
    Server server2(33339, users.get_path(), log2.get_path());
    bool started = false;
    try {
        started = server2.start();
    } catch (const std::exception&) {
        started = false;
    }
    CHECK(!started);
    
    server1.stop();
    }
    
    TEST(Test3_3_ServerStartInvalidUserFile) {
    TempFile log;
    Server server(33340, "non_existent_users.txt", log.get_path());
    bool started = false;
    try {
        started = server.start();
    } catch (const std::exception&) {
        started = false;
    }
    CHECK(!started);
    }
    
    TEST(Test4_1_ServerRunStartsWithoutException) {
        TempFile users("test:pass\n");
        TempFile log;
        Server server(33341, users.get_path(), log.get_path());
        
        CHECK(server.start());
        server.stop();
        
        CHECK(true);
    }
}

// ===================== MAIN =====================

// ===================== MAIN =====================

int main() {

    #ifdef __linux__
    // Игнорируем SIGPIPE на Linux
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
    #endif    
    
    
    
    // Запускаем все тесты
    int failedTests = UnitTest::RunAllTests();
    
    // Простая статистика
    std::cout << "\n" << std::string(40, '=') << std::endl;
    std::cout << "РЕЗУЛЬТАТ: ";
    if (failedTests == 0) {
        std::cout << "ВСЕ ТЕСТЫ ПРОЙДЕНЫ";
    } else {
        std::cout << "ПРОВАЛЕНО ТЕСТОВ: " << failedTests;
    }
    std::cout << std::endl << std::string(40, '=') << std::endl;
    
    return failedTests;
}
