# Projeto AED - roguelike top-down em C++ com SDL2
#
#   make                compila em build/ e gera o executavel ./jogo
#   make run            compila e executa
#   make clean          apaga os arquivos gerados
#   make format         aplica o estilo do .clang-format em src/
#   make format-check   so verifica, falha se algo estiver fora do padrao
#   make web            versao para navegador em build-web/ (precisa do em++ do emsdk)
#   make web-zip        compacta build-web/ em jogo-web.zip para enviar ao itch.io

CXX       := g++

DIR_INC   := src/include
DIR_SRC   := src/source
DIR_BUILD := build

CXXFLAGS  := -std=c++17 -Wall -Wextra -O2 -I$(DIR_INC) $(shell pkg-config --cflags sdl2 libpng)
LDLIBS    := $(shell pkg-config --libs sdl2 libpng)

ALVO      := jogo

CABECALHOS := $(shell find $(DIR_INC) -name '*.h')
FONTES    := $(shell find $(DIR_SRC) -name '*.cpp')
OBJETOS   := $(patsubst $(DIR_SRC)/%.cpp,$(DIR_BUILD)/%.o,$(FONTES))
DEPS      := $(OBJETOS:.o=.d)

CLANG_FORMAT := clang-format

# Versao web (Emscripten). So quem roda "make web" precisa do em++; o "make"
# normal continua usando g++ e as bibliotecas do sistema.
EMXX         := em++
DIR_WEB      := build-web
PORTAS_WEB   := -sUSE_SDL=2 -sUSE_LIBPNG=1 -sUSE_ZLIB=1
CXXFLAGS_WEB := -std=c++17 -Wall -Wextra -O2 -I$(DIR_INC) $(PORTAS_WEB)
# SINGLE_FILE + embed-file geram um index.html autossuficiente (codigo e assets
# embutidos), que abre ate com duplo clique (file://), sem servidor.
LDFLAGS_WEB  := $(PORTAS_WEB) -sALLOW_MEMORY_GROWTH=1 -lidbfs.js -sSINGLE_FILE=1 \
                --embed-file assets@/assets --shell-file web/shell.html
OBJETOS_WEB  := $(patsubst $(DIR_SRC)/%.cpp,$(DIR_WEB)/obj/%.o,$(FONTES))
ASSETS       := $(shell find assets -type f)

.PHONY: all run clean format format-check web web-zip

all: $(ALVO)

$(ALVO): $(OBJETOS)
	$(CXX) $(OBJETOS) -o $@ $(LDLIBS)

$(DIR_BUILD)/%.o: $(DIR_SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

web: $(DIR_WEB)/index.html

$(DIR_WEB)/index.html: $(OBJETOS_WEB) web/shell.html $(ASSETS)
	$(EMXX) $(OBJETOS_WEB) -o $@ $(LDFLAGS_WEB)

$(DIR_WEB)/obj/%.o: $(DIR_SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(EMXX) $(CXXFLAGS_WEB) -MMD -MP -c $< -o $@

# O itch.io espera um .zip com o index.html na raiz.
web-zip: web
	rm -f jogo-web.zip
	cd $(DIR_WEB) && python3 -m zipfile -c ../jogo-web.zip index.html

run: $(ALVO)
	./$(ALVO)

clean:
	rm -rf $(DIR_BUILD) $(ALVO) $(DIR_WEB) jogo-web.zip

format:
	$(CLANG_FORMAT) -i $(CABECALHOS) $(FONTES)

# Nao altera nada: so acusa quem esta fora do padrao, para rodar antes de commitar.
format-check:
	@$(CLANG_FORMAT) --dry-run --Werror $(CABECALHOS) $(FONTES) \
		&& echo "formatacao ok"

-include $(DEPS) $(OBJETOS_WEB:.o=.d)
