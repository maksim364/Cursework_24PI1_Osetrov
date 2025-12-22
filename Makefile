CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src
LDFLAGS = -lcryptopp -lboost_program_options
TEST_LDFLAGS = -lUnitTest++

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test
TEST_BUILD_DIR = $(BUILD_DIR)/test

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

TARGET = $(BUILD_DIR)/server
TEST_TARGET = $(TEST_BUILD_DIR)/run_tests

all: prepare $(TARGET)

prepare:
	@echo "Подготовка build директории..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/data
	@mkdir -p $(TEST_BUILD_DIR)
	@mkdir -p $(TEST_BUILD_DIR)/test_logs
	@cp -r data/* $(BUILD_DIR)/data/ 2>/dev/null || true
	@cp data/users.txt $(BUILD_DIR)/ 2>/dev/null || echo "Создайте data/users.txt для тестирования"
	@touch $(BUILD_DIR)/server.log 2>/dev/null || true
	@echo "Готово!"

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Сервер собран: $(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ========== Цели для тестирования ==========

test: prepare $(TEST_TARGET)
	@echo "Запуск модульных тестов..."
	@cd $(TEST_BUILD_DIR) && ./run_tests

test-build: prepare $(TEST_TARGET)
	@echo "Тесты собраны: $(TEST_TARGET)"

$(TEST_TARGET): $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LDFLAGS)

$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I./test -c $< -o $@

# Создание тестовых данных для тестов
test-data:
	@echo "Создание тестовых данных..."
	@mkdir -p $(TEST_BUILD_DIR)/data
	@echo "user1:pass1" > $(TEST_BUILD_DIR)/data/test_users_valid.txt
	@echo "user2:pass2" >> $(TEST_BUILD_DIR)/data/test_users_valid.txt
	@echo "admin:admin123" >> $(TEST_BUILD_DIR)/data/test_users_valid.txt
	@echo "test:test@123" >> $(TEST_BUILD_DIR)/data/test_users_valid.txt
	@echo "# Комментарий" > $(TEST_BUILD_DIR)/data/test_users_comments.txt
	@echo "user1:pass1" >> $(TEST_BUILD_DIR)/data/test_users_comments.txt
	@echo "" >> $(TEST_BUILD_DIR)/data/test_users_comments.txt
	@echo "user2:pass2" >> $(TEST_BUILD_DIR)/data/test_users_comments.txt
	@echo "  # Еще комментарий" >> $(TEST_BUILD_DIR)/data/test_users_comments.txt
	@touch $(TEST_BUILD_DIR)/data/test_users_empty.txt
	@echo "user1:pass1" > $(TEST_BUILD_DIR)/data/test_users_invalid.txt
	@echo "invalid_line_without_colon" >> $(TEST_BUILD_DIR)/data/test_users_invalid.txt
	@echo "user2:pass2" >> $(TEST_BUILD_DIR)/data/test_users_invalid.txt
	@echo ":emptypass" >> $(TEST_BUILD_DIR)/data/test_users_invalid.txt
	@echo "user3:" >> $(TEST_BUILD_DIR)/data/test_users_invalid.txt
	@echo "Тестовые данные созданы!"

# Быстрые тесты (без подготовки данных)
test-quick: $(TEST_TARGET)
	@echo "Быстрый запуск тестов..."
	@cd $(TEST_BUILD_DIR) && ./run_tests

# Запуск конкретного тестового сьюта
test-suite: test-build
	@if [ -z "$(suite)" ]; then \
		echo "Использование: make test-suite suite=SuiteName"; \
		echo "Доступные сьюты: CommandLineParserTests, LoggerTests, ErrorHandlerTests, AuthManagerTests, DataCalculatorTests, ServerTests"; \
		exit 1; \
	fi
	@echo "Запуск тестового сьюта: $(suite)"
	@cd $(TEST_BUILD_DIR) && ./run_tests $(suite)

# ========== Основные цели ==========

clean:
	rm -rf $(BUILD_DIR)
	@echo "Очистка завершена"

clean-tests:
	rm -rf $(TEST_BUILD_DIR)

debug: CXXFLAGS += -g -O0
debug: all
	@echo "Собран debug-версия сервера"

help: all
	cd $(BUILD_DIR) && ./server --help
	

.PHONY: all prepare clean clean-tests debug help test test-build test-data test-quick test-suite 
