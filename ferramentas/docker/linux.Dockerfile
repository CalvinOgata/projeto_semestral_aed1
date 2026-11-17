# Ambiente para "make dist-linux". Ubuntu 22.04 (glibc 2.35): o binario gerado
# roda nessa versao e nas mais novas. Os pacotes -dev abaixo sao so cabecalhos
# para o SDL2 de externo/ saber falar com X11, Wayland, audio e controles; no
# computador do jogador essas bibliotecas sao abertas pelo SDL se existirem.
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config python3 \
        libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
        libxfixes-dev libxinerama-dev libxss-dev libxxf86vm-dev libxkbcommon-dev \
        libwayland-dev wayland-protocols libdecor-0-dev \
        libgl-dev libegl-dev libgles-dev \
        libasound2-dev libpulse-dev libpipewire-0.3-dev \
        libdbus-1-dev libudev-dev \
    && rm -rf /var/lib/apt/lists/*
