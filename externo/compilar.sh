#!/bin/sh
# Compila zlib, libpng e SDL2 a partir dos .tar.gz desta pasta, como bibliotecas
# estaticas. Assim o jogo nao depende dessas bibliotecas instaladas no sistema e
# o executavel final ja leva tudo dentro.
#
#   uso: externo/compilar.sh <linux|windows> <diretorio_de_saida>
#
# Resultado: <diretorio_de_saida>/instalado/{include,lib}. O Makefile chama este
# script sozinho na primeira compilacao; nao e preciso roda-lo a mao.
set -eu

SO=$1
SAIDA=$(mkdir -p "$2" && cd "$2" && pwd)
EXTERNO=$(cd "$(dirname "$0")" && pwd)

FONTES=$SAIDA/fontes
OBRA=$SAIDA/obra
PREFIXO=$SAIDA/instalado
NUCLEOS=$(nproc 2>/dev/null || echo 4)
# O cmake --build tem paralelismo proprio; nao herda o -j do make externo.
unset MAKEFLAGS MFLAGS

case $SO in
linux)
    CRUZADO=""
    ;;
windows)
    # Compilacao cruzada: roda no Linux e gera codigo para Windows (MinGW-w64).
    CRUZADO="-DCMAKE_SYSTEM_NAME=Windows
             -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
             -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
             -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres
             -DCMAKE_FIND_ROOT_PATH=$PREFIXO
             -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER
             -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY
             -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY"
    ;;
*)
    echo "uso: $0 <linux|windows> <diretorio_de_saida>" >&2
    exit 1
    ;;
esac

# Garante que os pacotes nao foram corrompidos ou trocados.
(cd "$EXTERNO" && sha256sum -c --quiet SHA256SUMS)

rm -rf "$FONTES" "$OBRA" "$PREFIXO"
mkdir -p "$FONTES" "$OBRA"
for pacote in "$EXTERNO"/*.tar.gz; do
    tar -xzf "$pacote" -C "$FONTES"
done

# $CRUZADO fica sem aspas de proposito: cada linha vira um argumento.
compilar() {
    nome=$1
    shift
    # shellcheck disable=SC2086
    cmake -S "$FONTES/$nome" -B "$OBRA/$nome" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIXO" \
        -DCMAKE_INSTALL_LIBDIR=lib \
        $CRUZADO "$@"
    cmake --build "$OBRA/$nome" -j "$NUCLEOS"
    cmake --install "$OBRA/$nome"
}

compilar zlib-1.3.2 \
    -DZLIB_BUILD_SHARED=OFF -DZLIB_BUILD_STATIC=ON -DZLIB_BUILD_TESTING=OFF
# No Windows a zlib chama a versao estatica de libzs.a; padroniza o nome.
if [ -f "$PREFIXO/lib/libzs.a" ]; then
    cp "$PREFIXO/lib/libzs.a" "$PREFIXO/lib/libz.a"
fi

compilar libpng-1.6.59 \
    -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_TOOLS=OFF \
    -DPNG_EXECUTABLES=OFF -DPNG_FRAMEWORK=OFF \
    -DZLIB_INCLUDE_DIR="$PREFIXO/include" -DZLIB_LIBRARY="$PREFIXO/lib/libz.a"

# No Linux o SDL continua abrindo X11/Wayland/PulseAudio/OpenGL do sistema em
# tempo de execucao (dlopen), o que todo desktop Linux ja tem.
compilar SDL2-2.32.10 \
    -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF

rm -rf "$FONTES" "$OBRA"
touch "$SAIDA/pronto"
