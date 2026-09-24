# Makefile

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# Caminho da SFML baixada
SFML_DIR := external/SFML-3.1.0
SFML_INC := $(SFML_DIR)/include
SFML_LIB := $(SFML_DIR)/lib

# Includes e libs
INCLUDES := -I$(SFML_INC) -I include
LIBS := -L$(SFML_LIB) -lsfml-graphics -lsfml-window -lsfml-system
RPATH := -Wl,-rpath,'$$ORIGIN/$(SFML_LIB)'

# Arquivos fonte e objeto
SRC_DIR := src
BUILD_DIR := build
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET := jogo

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LIBS) $(RPATH)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)