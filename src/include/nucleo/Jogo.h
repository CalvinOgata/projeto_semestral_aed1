#ifndef JOGO_H
#define JOGO_H

#include <SDL2/SDL.h>

#include "entidades/Jogador.h"
#include "graficos/Camera.h"
#include "interface/Menu.h"
#include "mundo/Campanha.h"
#include "persistencia/ArquivoProgresso.h"

enum class EstadoJogo
{
    INICIAL,
    VITORIA,
    JOGANDO,
    PAUSADO,
    SAINDO
};

class Jogo
{
private:
    SDL_Window *janela;
    SDL_Renderer *renderizador;

    EstadoJogo estado;
    Jogador jogador;
    Menu menu;
    Camera camera;

    Campanha campanha;
    ArquivoProgresso progresso;
    bool savePendente = false;
    std::string mensagemSave;

    Uint64 tempoAnterior;
    float esperaTiro; // segundos que faltam para o proximo disparo
    bool atirando;    // botao esquerdo do mouse segurado
    float mouseTelaX; // posicao do mouse em coordenadas de tela
    float mouseTelaY;

public:
    Jogo();
    ~Jogo();

    // A janela e o renderizador SDL possuem um unico dono.
    Jogo(const Jogo &) = delete;
    Jogo &operator=(const Jogo &) = delete;

    bool inicializar();
    void executar();

    EstadoJogo get_estado() const;
    int get_inimigosVivos() const;

private:
    void tratar_eventos();
    void atualizar(float dt);
    void desenhar();

    void mover_jogador(float dt);
    void atualizar_mira();

    void atualizar_camera();

    void abrir_inicio();
    void iniciar_partida(bool continuar);
    void salvar_progresso();
    void preparar_entrada();
    void pausar();
    void retomar();
    void finalizar();
};

#endif
