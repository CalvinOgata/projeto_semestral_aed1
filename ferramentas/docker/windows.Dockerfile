# Ambiente para "make dist-windows": compila no Linux gerando um .exe com o
# MinGW-w64 (compilacao cruzada). Nao precisa de Windows para compilar.
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        make cmake pkg-config python3 \
        gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64 \
    && rm -rf /var/lib/apt/lists/*
