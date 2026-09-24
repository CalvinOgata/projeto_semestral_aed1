#ifndef JOGADOR_H
#define JOGADOR_H

#include <SDL2/SDL.h>

#include "FilaArmas.h"

class Jogador{
private:
    int vida;
    int estamina;
    int vidaMax;
    int estaminaMax;

    float x;
    float y;
    float largura;
    float altura;
    float velocidade;   // pixels por segundo

    FilaArmas armas;

public:
    Jogador();
    ~Jogador();

    // --- atributos ---
    void rec_vida(int);
    void rec_estamina(int);
    void perder_vida(int);
    void perder_estamina(int);
    void mudar_vidaMax(int);
    void mudar_estaminaMax(int);

    int get_estamina() const;
    int get_vida() const;
    int get_vidaMax() const;
    int get_estaminaMax() const;
    bool Morto() const;

    // --- posicao e movimento ---
    void set_posicao(float, float);
    void set_velocidade(float);
    void mover(int dirX, int dirY, float dt);
    void manter_nos_limites(const SDL_Rect &limites);

    float get_x() const;
    float get_y() const;
    float get_largura() const;
    float get_altura() const;
    float get_velocidade() const;
    SDL_FRect get_retangulo() const;

    // --- inventario ---
    FilaArmas& get_armas();
    const FilaArmas& get_armas() const;

    void desenhar(SDL_Renderer*) const;
};

#endif
