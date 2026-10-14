#ifndef JOGO_H
#define JOGO_H

#include <SDL2/SDL.h>

#include <vector>

#include "Camera.h"
#include "Inimigo.h"
#include "Jogador.h"
#include "Menu.h"
#include "Projetil.h"

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
    Camera     camera;

    std::vector<Projetil> projeteis;
    std::vector<Inimigo>  inimigos;

    Uint64 tempoAnterior;
    float  esperaTiro;    // segundos que faltam para o proximo disparo
    bool   atirando;      // botao esquerdo do mouse segurado
    float  mouseTelaX;    // posicao do mouse em coordenadas de tela
    float  mouseTelaY;

public:
    Jogo();
    ~Jogo();

    bool inicializar();
    void executar();

    EstadoJogo get_estado() const;
    int get_inimigos_vivos() const;

private:
    void tratar_eventos();
    void atualizar(float dt);
    void desenhar();

    void mover_jogador(float dt);
    void atualizar_mira();
    void disparar();
    void atualizar_projeteis(float dt);
    void atualizar_inimigos(float dt);
    void resolver_colisoes();
    void limpar_inativos();

    void povoar_inimigos();
    void desenhar_grade();

    void pausar();
    void retomar();
    void finalizar();
};

#endif
