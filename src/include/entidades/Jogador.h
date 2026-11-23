#ifndef JOGADOR_H
#define JOGADOR_H

#include <SDL2/SDL.h>

#include "config/DimensoesEntidade.h"
#include "graficos/Camera.h"
#include "inventario/FilaArmas.h"

class Sprite;

class Jogador
{
private:
    float esperaEscudo = 0;
    float cargaEscudo = 0;
    float invulnerabilidade = 0;
    bool ignorarTirosInimigos = false;
    int vida;
    int escudo;
    int escudoMax;
    int vidaMax;

    float x;
    float y;
    DimensoesEntidade dimensoes;
    float velocidade; // pixels por segundo
    float anguloMira; // graus, 0 = apontando para a direita
    float tempoAnimacao = 0;

    FilaArmas armas;

public:
    explicit Jogador(const DimensoesEntidade &dimensoes = DIMENSOES_HEROI);
    ~Jogador();

    // --- atributos ---
    void receber_dano(int quantidade);
    void atualizar_defesa(float dt);
    void rec_vida(int);
    void rec_escudo(int);
    void perder_escudo(int);
    int get_escudo() const;
    int get_escudoMax() const;
    void perder_vida(int);
    void mudar_vidaMax(int);

    int get_vida() const;
    int get_vidaMax() const;
    bool morto() const;
    void set_ignorarTirosInimigos(bool ignorar);
    bool get_ignorarTirosInimigos() const;

    // --- posicao e movimento ---
    void set_posicao(float, float);
    void set_velocidade(float);
    void mover(int dirX, int dirY, float dt);
    // Joystick de toque: vetor com comprimento ate 1 (meio caminho = meia velocidade).
    void mover_analogico(float dirX, float dirY, float dt);

    float get_x() const;
    float get_y() const;
    float get_centroX() const;
    float get_centroY() const;
    float get_largura() const;
    float get_altura() const;
    float get_velocidade() const;
    SDL_FRect get_retangulo() const; // hitbox: colisao, portas e limites
    SDL_FRect get_retanguloVisual() const; // destino do sprite em coordenadas de mundo

    // --- mira e arma ---
    void mirar_em(float mundoX, float mundoY);
    float get_anguloMira() const;
    SDL_FPoint get_direcaoMira() const;
    SDL_FPoint get_bocaArma() const; // de onde o projetil sai
    Armas *get_armaEquipada();       // nullptr se a fila estiver vazia

    // --- inventario ---
    FilaArmas &get_armas();
    const FilaArmas &get_armas() const;

    void desenhar(SDL_Renderer *, const Camera &, bool mostrarHitbox = false,
                  const Sprite *sprite = nullptr) const;
};

#endif
