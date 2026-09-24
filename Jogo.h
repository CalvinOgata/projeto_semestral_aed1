#ifndef JOGO_H
#define JOGO_H

#include <SDL2/SDL.h>

#include "Jogador.h"
#include "Menu.h"

enum class EstadoJogo
{
    JOGANDO,
    PAUSADO,
    SAINDO
};

class Jogo{
private:
    SDL_Window   *janela;
    SDL_Renderer *renderizador;

    EstadoJogo estado;
    Jogador    jogador;
    Menu       menu;
    SDL_Rect   arena;
    Uint64     tempoAnterior;

public:
    Jogo();
    ~Jogo();

    bool inicializar();
    void executar();

    EstadoJogo get_estado() const;

private:
    void tratar_eventos();
    void atualizar(float dt);
    void desenhar();

    void desenhar_arena();
    void desenhar_hud();

    void pausar();
    void retomar();
    void finalizar();
};

#endif
