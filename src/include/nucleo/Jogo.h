#ifndef JOGO_H
#define JOGO_H

#include <SDL2/SDL.h>

#include "entidades/Jogador.h"
#include "graficos/Camera.h"
#include "graficos/Sprite.h"
#include "interface/Menu.h"
#include "interface/TelaCarregamento.h"
#include "interface/TelaDerrota.h"
#include "interface/TelaVitoria.h"
#include "mundo/Campanha.h"
#include "persistencia/ArquivoConfiguracoes.h"
#include "persistencia/ArquivoProgresso.h"

enum class EstadoJogo
{
    CARREGANDO,
    INICIAL,
    VITORIA,
    RESUMO_FASE,
    GAME_OVER,
    JOGANDO,
    DERROTA, // animacao do sistema invadido antes de reiniciar a fase
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
    TelaCarregamento carregamento;
    TelaDerrota derrota;
    TelaVitoria vitoria;
    Configuracoes configuracoes;
    ArquivoConfiguracoes arquivoConfiguracoes;
    Camera camera;
    Sprite spriteJogador;
    Sprite spriteGameOver;

    Campanha campanha;
    ArquivoProgresso progresso;
    int slotAtivo = 0;
    bool selecionandoNovoJogo = false;
    bool savePendente = false;
    std::string mensagemSave;

    int vidas = VIDAS_INICIAIS;
    bool continueDisponivel = true;
    int pontuacaoTotal = 0;
    int pontosComunsFase = 0;
    int pontosBossFase = 0;
    int pontosComunsObservados = 0;
    int pontosBossObservados = 0;
    float tempoFase = 0.0f;
    struct ResumoFase
    {
        int numero = 1;
        int pontosComuns = 0;
        int pontosBoss = 0;
        int pontosRapidez = 0;
        int total = 0;
        float tempo = 0.0f;
    } resumoFase;
    float tempoGameOver = 0.0f;

    bool cheatTiros = false;
    int progressoCheat = 0;
    float tempoMensagemCheat = 0.0f;

    Uint64 tempoAnterior;
    float esperaTiro; // segundos que faltam para o proximo disparo
    bool mostrarMinimapa = false;
    float tempoTituloFase = 0.0f; // segundos restantes do titulo ao entrar na fase
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
    void executar(); // laco do desktop: repete passo() ate o jogo sair
    void passo();    // um quadro: eventos, atualizacao e desenho

    EstadoJogo get_estado() const;
    int get_inimigosVivos() const;

private:
    void tratar_eventos();
    void atualizar(float dt);
    void atualizar_gameOver(float dt);
    void desenhar();
    void desenhar_resumoFase() const;
    void desenhar_gameOver() const;

    void mover_jogador(float dt);
    void atualizar_mira();

    void atualizar_camera();

    void atualizar_carregamento(float dt);
    void atualizar_musica();
    void aplicar_configuracoes();
    void salvar_configuracoes();

    void abrir_inicio(bool telaJogar = false);
    void abrir_selecaoSlots(bool novoJogo);
    void iniciar_partida(bool continuar, int slot);
    void salvar_progresso();
    void concluir_fase();
    void iniciar_derrota();
    void concluir_derrota();
    void iniciar_gameOver();
    void aceitar_continue();
    void encerrar_gameOver();
    void registrar_cheat(SDL_Keycode tecla);
    void atualizar_pontuacaoInimigos();
    void preparar_entrada();
    void pausar();
    void retomar();
    void finalizar();
};

#endif
