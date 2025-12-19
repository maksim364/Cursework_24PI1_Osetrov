CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src
LDFLAGS = -lcryptopp -lboost_program_options

BUILD_DIR = build
SRC_DIR = src

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = $(BUILD_DIR)/server

all: prepare $(TARGET)

prepare:
	@echo "Подготовка build директории..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/data
	@cp -r data/* $(BUILD_DIR)/data/ 2>/dev/null || true
	@cp data/users.txt $(BUILD_DIR)/ 2>/dev/null || echo "Создайте data/users.txt для тестирования"
	@touch $(BUILD_DIR)/server.log 2>/dev/null || true
	@echo "Готово!"

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Сервер собран: $(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "Очистка завершена"

run: all
	@echo "🚀 Запуск сервера..."
	cd $(BUILD_DIR) && ./server

help: all
	cd $(BUILD_DIR) && ./server --help

.PHONY: all prepare clean run help
