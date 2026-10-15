#ifndef JOGADOR_H
#define JOGADOR_H

#include <SDL2/SDL.h>

#include "graficos/Camera.h"
#include "inventario/FilaArmas.h"

class Jogador
{
private:
    int vida;
    int escudo;
    int escudoMax;
    int estamina;
    int vidaMax;
    int estaminaMax;

    float x;
    float y;
    float largura;
    float altura;
    float velocidade; // pixels por segundo
    float anguloMira; // graus, 0 = apontando para a direita

    FilaArmas armas;

public:
    Jogador();
    ~Jogador();

    // --- atributos ---
    void rec_vida(int);
    void rec_escudo(int);
    void perder_escudo(int);
    int get_escudo() const;
    int get_escudoMax() const;
    void rec_estamina(int);
    void perder_vida(int);
    void perder_estamina(int);
    void mudar_vidaMax(int);
    void mudar_estaminaMax(int);

    int get_estamina() const;
    int get_vida() const;
    int get_vidaMax() const;
    int get_estaminaMax() const;
    bool morto() const;

    // --- posicao e movimento ---
    void set_posicao(float, float);
    void set_velocidade(float);
    void mover(int dirX, int dirY, float dt);

    float get_x() const;
    float get_y() const;
    float get_centroX() const;
    float get_centroY() const;
    float get_largura() const;
    float get_altura() const;
    float get_velocidade() const;
    SDL_FRect get_retangulo() const;

    // --- mira e arma ---
    void mirar_em(float mundoX, float mundoY);
    float get_anguloMira() const;
    SDL_FPoint get_direcaoMira() const;
    SDL_FPoint get_bocaArma() const; // de onde o projetil sai
    Armas *get_armaEquipada();       // nullptr se a fila estiver vazia

    // --- inventario ---
    FilaArmas &get_armas();
    const FilaArmas &get_armas() const;

    void desenhar(SDL_Renderer *, const Camera &) const;
};

#endif
