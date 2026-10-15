#ifndef INIMIGO_H
#define INIMIGO_H

#include <SDL2/SDL.h>

#include "graficos/Camera.h"

class Inimigo
{
private:
    float x; // canto superior esquerdo, em coordenadas de mundo
    float y;
    float lado;
    float velocidade;
    int vida;
    bool vivo;

public:
    Inimigo();
    Inimigo(float mundoX, float mundoY);

    Inimigo(float mundoX, float mundoY, int vidaInicial, float velocidadeInicial);
    void limitar_a(const SDL_FRect &area);

    void perseguir(float alvoX, float alvoY, float dt);
    void afastar_de(Inimigo &outro);
    void levar_dano(int);

    bool get_vivo() const;
    int get_vida() const;

    float get_centroX() const;
    float get_centroY() const;
    float get_lado() const;
    SDL_FRect get_retangulo() const;

    void desenhar(SDL_Renderer *, const Camera &) const;
};

#endif
