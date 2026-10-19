#ifndef SALA_H
#define SALA_H

#include <string>
#include <array>

#include "sistemas/Combate.h"

enum class Direcao { ESQUERDA, DIREITA, CIMA, BAIXO };

struct RequisitoSala
{
    bool eliminarInimigos = true;
};

struct ConfiguracaoSala
{
    std::string nome;
    float largura;
    float altura;
    int dificuldade;
    int quantidadeInimigos;
    RequisitoSala requisito;
    int brutos = 0;
    bool boss = false;
    int baus = 0;
    bool armasIniciais = true;
    std::array<int, 4> vizinhos = {{-1, -1, -1, -1}};
    bool mapaRamificado = false;
    bool portalSaida = false;
};

enum class EstadoSala
{
    NAO_VISITADA,
    EM_COMBATE,
    LIBERADA
};

// Cada sala possui suas entidades e progresso, inclusive ao ser revisitada.
class Sala
{
private:
    ConfiguracaoSala configuracao;
    Combate combate;
    struct ArmaChao
    {
        Armas arma;
        float x, y;
        bool coletada = false;
    };
    ListaSequencial<ArmaChao> armasChao;
    struct Bau { float x, y; bool aberto = false; };
    ListaSequencial<Bau> baus;
    bool preparada = false;
    EstadoSala estado = EstadoSala::NAO_VISITADA;

public:
    explicit Sala(const ConfiguracaoSala &configuracao);
    void entrar(Jogador &jogador, bool pelaDireita = false);
    void entrar(Jogador &jogador, Direcao entrada);
    // Preparar cria entidades sem visitar a sala nem iniciar sua simulacao.
    void preparar();
    void ativar();
    bool porta_liberada(Direcao direcao) const;
    static constexpr float COMPRIMENTO_CORREDOR = 240;
    SDL_FRect get_corredor(Direcao direcao) const;
    SDL_FRect get_porta(Direcao direcao) const;
    bool tocando_porta(const Jogador &jogador, Direcao direcao) const;
    bool tocando_portal(const Jogador &jogador) const;
    void sair();
    void envelhecer_coletas(float dt);
    void atualizar(float dt, Jogador &jogador);
    void disparar(Jogador &jogador);
    bool coletar_arma(Jogador &jogador);
    void soltar_arma(Jogador &jogador);
    void limitar_jogador(Jogador &jogador) const;
    bool tocando_porta(const Jogador &jogador, bool direita) const;
    bool liberada() const;
    EstadoSala get_estado() const;
    int get_inimigosVivos() const;
    const ConfiguracaoSala &get_configuracao() const;
    SDL_FRect get_limites() const;
    SDL_FRect get_porta(bool direita) const;
    void desenhar_corredores(SDL_Renderer *renderizador, const Camera &camera) const;
    void desenhar(SDL_Renderer *renderizador, const Camera &camera, bool temAnterior,
                  bool mostrarHitboxes = false) const;
};

#endif
