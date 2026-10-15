#ifndef SALA_H
#define SALA_H

#include <string>

#include "sistemas/Combate.h"

struct RequisitoSala
{
    bool eliminarInimigos = true;
    float tempoMinimo = 0.0f; // segundos ativos dentro da sala; pausa nao conta
};

struct ConfiguracaoSala
{
    std::string nome;
    float largura;
    float altura;
    int dificuldade;
    int quantidadeInimigos;
    RequisitoSala requisito;
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
    EstadoSala estado = EstadoSala::NAO_VISITADA;
    float tempoAtivo = 0.0f;

public:
    explicit Sala(const ConfiguracaoSala &configuracao);
    void entrar(Jogador &jogador, bool pelaDireita = false);
    void sair();
    void atualizar(float dt, const Jogador &jogador);
    void disparar(Jogador &jogador);
    void limitar_jogador(Jogador &jogador) const;
    bool perto_porta(const Jogador &jogador, bool direita) const;
    bool liberada() const;
    EstadoSala get_estado() const;
    int get_inimigosVivos() const;
    float get_tempoRestante() const;
    const ConfiguracaoSala &get_configuracao() const;
    SDL_FRect get_limites() const;
    SDL_FRect get_porta(bool direita) const;
    void desenhar(SDL_Renderer *renderizador, const Camera &camera, bool temAnterior) const;
};

#endif
