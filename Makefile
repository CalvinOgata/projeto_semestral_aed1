# Projeto AED - roguelike top-down em C++ com SDL2
#
#   make                compila em build/ e gera o executavel ./jogo
#   make run            compila e executa
#   make clean          apaga os arquivos gerados
#   make format         aplica o estilo do .clang-format em src/
#   make format-check   so verifica, falha se algo estiver fora do padrao

CXX       := g++

DIR_INC   := src/include
DIR_SRC   := src/source
DIR_BUILD := build

CXXFLAGS  := -std=c++17 -Wall -Wextra -O2 -I$(DIR_INC) $(shell pkg-config --cflags sdl2)
LDLIBS    := $(shell pkg-config --libs sdl2)

ALVO      := jogo

CABECALHOS := $(wildcard $(DIR_INC)/*.h)
FONTES    := $(wildcard $(DIR_SRC)/*.cpp)
OBJETOS   := $(patsubst $(DIR_SRC)/%.cpp,$(DIR_BUILD)/%.o,$(FONTES))
DEPS      := $(OBJETOS:.o=.d)

CLANG_FORMAT := clang-format

.PHONY: all run clean format format-check

all: $(ALVO)

$(ALVO): $(OBJETOS)
	$(CXX) $(OBJETOS) -o $@ $(LDLIBS)

$(DIR_BUILD)/%.o: $(DIR_SRC)/%.cpp | $(DIR_BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(DIR_BUILD):
	mkdir -p $(DIR_BUILD)

run: $(ALVO)
	./$(ALVO)

clean:
	rm -rf $(DIR_BUILD) $(ALVO)

format:
	$(CLANG_FORMAT) -i $(CABECALHOS) $(FONTES)

# Nao altera nada: so acusa quem esta fora do padrao, para rodar antes de commitar.
format-check:
	@$(CLANG_FORMAT) --dry-run --Werror $(CABECALHOS) $(FONTES) \
		&& echo "formatacao ok"

-include $(DEPS)
