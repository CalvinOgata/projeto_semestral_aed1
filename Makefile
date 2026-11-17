# Projeto AED - roguelike top-down em C++ com SDL2
#
#   make                compila e gera o executavel ./jogo (Linux)
#   make run            compila e executa
#   make dist-linux     gera dist/jogo para distribuir (via Docker, Ubuntu 22.04)
#   make dist-windows   gera dist/jogo.exe para distribuir (via Docker, MinGW-w64)
#   make dist           as duas distribuicoes acima
#   make clean          apaga o codigo compilado do jogo e os executaveis
#   make clean-tudo     tambem apaga as bibliotecas ja compiladas de externo/
#   make format         aplica o estilo do .clang-format em src/
#   make format-check   so verifica, falha se algo estiver fora do padrao
#   make web            versao para navegador em build-web/ (precisa do em++ do emsdk)
#   make web-zip        compacta build-web/ em jogo-web.zip para enviar ao itch.io
#
# SDL2, libpng e zlib vem de externo/ (codigo-fonte das versoes fixadas) e sao
# compiladas como bibliotecas estaticas na primeira compilacao (leva ~1 minuto).
# Os sprites usados sao copiados para dentro do executavel. Assim o resultado e
# um unico arquivo, sem DLLs, .so ou pasta assets/ ao lado.
#
# "make ALVO_SO=windows" compila para Windows sem Docker, se o MinGW-w64
# (x86_64-w64-mingw32-g++) estiver instalado.

ALVO_SO   ?= linux
DIR_BUILD ?= build/$(ALVO_SO)

ifeq ($(ALVO_SO),windows)
CXX        := x86_64-w64-mingw32-g++
SAIDA      ?= jogo.exe
# M_PI nao e C++ padrao; o glibc ja o expoe, o MinGW so com esta definicao.
CXXFLAGS_SO := -D_USE_MATH_DEFINES
# -static embute libstdc++, libgcc e winpthread; -mwindows evita o console.
LDFLAGS_SO := -static -mwindows
else ifeq ($(ALVO_SO),linux)
CXX        := g++
SAIDA      ?= jogo
CXXFLAGS_SO :=
# A glibc continua dinamica (existe em toda distro); o runtime de C++ vai junto.
LDFLAGS_SO := -static-libstdc++ -static-libgcc
else
$(error ALVO_SO deve ser linux ou windows)
endif

DIR_INC   := src/include
DIR_SRC   := src/source

# Bibliotecas de externo/, compiladas por externo/compilar.sh.
DIR_EXTERNO    := $(DIR_BUILD)/externo
PREFIXO        := $(DIR_EXTERNO)/instalado
EXTERNO_PRONTO := $(DIR_EXTERNO)/pronto
PACOTES        := $(wildcard externo/*.tar.gz)
# Le so o sdl2.pc compilado aqui, nunca o SDL2 instalado no sistema.
PKG_SDL         = PKG_CONFIG_LIBDIR=$(abspath $(PREFIXO))/lib/pkgconfig pkg-config

# "=" (e nao ":=") porque o pkg-config so pode rodar depois que o SDL2 existir.
CXXFLAGS   = -std=c++17 -Wall -Wextra -O2 -I$(DIR_INC) $(CXXFLAGS_SO) $(shell $(PKG_SDL) --cflags sdl2)
LDLIBS     = $(PREFIXO)/lib/libpng16.a $(PREFIXO)/lib/libz.a \
             $(shell $(PKG_SDL) --static --libs sdl2)

CABECALHOS := $(shell find $(DIR_INC) -name '*.h')
FONTES     := $(shell find $(DIR_SRC) -name '*.cpp')
OBJETOS    := $(patsubst $(DIR_SRC)/%.cpp,$(DIR_BUILD)/obj/%.o,$(FONTES))
DEPS       := $(OBJETOS:.o=.d)

# Assets copiados para dentro do executavel: NOME=caminho. Cada NOME tambem
# precisa estar declarado em src/include/graficos/AssetsEmbutidos.h.
ASSETS_EMBUTIDOS := SPRITE_JOGADOR=assets/sprites/personagens/tux-pixel-v2.png \
                    SPRITE_GAME_OVER=assets/sprites/personagens/tux-gameover-v2.png
ARQUIVOS_EMBUTIDOS := $(foreach par,$(ASSETS_EMBUTIDOS),$(word 2,$(subst =, ,$(par))))
EMBUTIR            := python3 ferramentas/embutir_assets.py
GERADO             := $(DIR_BUILD)/gerado/AssetsEmbutidos.cpp
OBJETO_GERADO      := $(DIR_BUILD)/gerado/AssetsEmbutidos.o

CLANG_FORMAT := clang-format

# Versao web (Emscripten). So quem roda "make web" precisa do em++. Os ports do
# Emscripten ja compilam SDL2/libpng/zlib para dentro do index.html.
EMXX          := em++
DIR_WEB       := build-web
PORTAS_WEB    := -sUSE_SDL=2 -sUSE_LIBPNG=1 -sUSE_ZLIB=1
CXXFLAGS_WEB  := -std=c++17 -Wall -Wextra -O2 -I$(DIR_INC) $(PORTAS_WEB)
# SINGLE_FILE gera um index.html autossuficiente que abre ate com duplo clique
# (file://), sem servidor. Os sprites ja vao embutidos no codigo.
LDFLAGS_WEB   := $(PORTAS_WEB) -sALLOW_MEMORY_GROWTH=1 -lidbfs.js -sSINGLE_FILE=1 \
                 --shell-file web/shell.html
OBJETOS_WEB   := $(patsubst $(DIR_SRC)/%.cpp,$(DIR_WEB)/obj/%.o,$(FONTES))
GERADO_WEB    := $(DIR_WEB)/gerado/AssetsEmbutidos.cpp
OBJ_GERADO_WEB := $(DIR_WEB)/gerado/AssetsEmbutidos.o

# Builds de distribuicao dentro de containers, para nao depender do sistema de
# quem compila. O Linux usa Ubuntu 22.04 de proposito: um binario feito com uma
# glibc antiga roda nas distros mais novas, mas o contrario nao.
DOCKER_RODAR = docker run --rm -u "$$(id -u):$$(id -g)" -e HOME=/tmp \
               -v "$(CURDIR)":/projeto -w /projeto

.PHONY: all run clean clean-tudo format format-check web web-zip dist dist-linux dist-windows

all: $(SAIDA)

$(SAIDA): $(OBJETOS) $(OBJETO_GERADO)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJETOS) $(OBJETO_GERADO) -o $@ $(LDFLAGS_SO) $(LDLIBS)

# Os objetos precisam dos cabecalhos do SDL2 compilado em externo/.
$(DIR_BUILD)/obj/%.o: $(DIR_SRC)/%.cpp | $(EXTERNO_PRONTO)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(EXTERNO_PRONTO): externo/compilar.sh externo/SHA256SUMS $(PACOTES)
	sh externo/compilar.sh $(ALVO_SO) $(DIR_EXTERNO)

$(GERADO): ferramentas/embutir_assets.py $(ARQUIVOS_EMBUTIDOS) Makefile
	$(EMBUTIR) $@ $(ASSETS_EMBUTIDOS)

$(OBJETO_GERADO): $(GERADO) | $(EXTERNO_PRONTO)
	$(CXX) $(CXXFLAGS) -c $< -o $@

dist: dist-linux dist-windows

dist-linux:
	docker build -t aed1-dist-linux -f ferramentas/docker/linux.Dockerfile ferramentas/docker
	$(DOCKER_RODAR) aed1-dist-linux \
		sh -c 'make -j"$$(nproc)" ALVO_SO=linux DIR_BUILD=build/dist-linux SAIDA=dist/jogo'

dist-windows:
	docker build -t aed1-dist-windows -f ferramentas/docker/windows.Dockerfile ferramentas/docker
	$(DOCKER_RODAR) aed1-dist-windows \
		sh -c 'make -j"$$(nproc)" ALVO_SO=windows DIR_BUILD=build/dist-windows SAIDA=dist/jogo.exe'

web: $(DIR_WEB)/index.html

$(DIR_WEB)/index.html: $(OBJETOS_WEB) $(OBJ_GERADO_WEB) web/shell.html
	$(EMXX) $(OBJETOS_WEB) $(OBJ_GERADO_WEB) -o $@ $(LDFLAGS_WEB)

$(DIR_WEB)/obj/%.o: $(DIR_SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(EMXX) $(CXXFLAGS_WEB) -MMD -MP -c $< -o $@

$(GERADO_WEB): ferramentas/embutir_assets.py $(ARQUIVOS_EMBUTIDOS) Makefile
	$(EMBUTIR) $@ $(ASSETS_EMBUTIDOS)

$(OBJ_GERADO_WEB): $(GERADO_WEB)
	$(EMXX) $(CXXFLAGS_WEB) -c $< -o $@

# O itch.io espera um .zip com o index.html na raiz.
web-zip: web
	rm -f jogo-web.zip
	cd $(DIR_WEB) && python3 -m zipfile -c ../jogo-web.zip index.html

run: $(SAIDA)
	./$(SAIDA)

# Mantem build/*/externo: recompilar o SDL2 a cada clean seria lento a toa.
clean:
	rm -rf build/*/obj build/*/gerado jogo jogo.exe dist $(DIR_WEB) jogo-web.zip

clean-tudo: clean
	rm -rf build

format:
	$(CLANG_FORMAT) -i $(CABECALHOS) $(FONTES)

# Nao altera nada: so acusa quem esta fora do padrao, para rodar antes de commitar.
format-check:
	@$(CLANG_FORMAT) --dry-run --Werror $(CABECALHOS) $(FONTES) \
		&& echo "formatacao ok"

-include $(DEPS) $(OBJETOS_WEB:.o=.d)
