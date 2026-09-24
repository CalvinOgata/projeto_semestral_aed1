# Projeto AED - roguelike top-down em C++ com SDL2
#
#   make          compila em build/ e gera o executavel ./jogo
#   make run      compila e executa
#   make clean    apaga os arquivos gerados

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 $(shell pkg-config --cflags sdl2)
LDLIBS   := $(shell pkg-config --libs sdl2)

ALVO     := jogo
DIR_BUILD := build

FONTES   := $(wildcard *.cpp)
OBJETOS  := $(patsubst %.cpp,$(DIR_BUILD)/%.o,$(FONTES))
DEPS     := $(OBJETOS:.o=.d)

.PHONY: all run clean

all: $(ALVO)

$(ALVO): $(OBJETOS)
	$(CXX) $(OBJETOS) -o $@ $(LDLIBS)

$(DIR_BUILD)/%.o: %.cpp | $(DIR_BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(DIR_BUILD):
	mkdir -p $(DIR_BUILD)

run: $(ALVO)
	./$(ALVO)

clean:
	rm -rf $(DIR_BUILD) $(ALVO)

-include $(DEPS)
