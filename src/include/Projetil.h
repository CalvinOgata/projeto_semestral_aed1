#ifndef PROJETIL_H
#define PROJETIL_H

#include <SDL2/SDL.h>

#include "Camera.h"

class Projetil{
private:
    float x;    // centro, em coordenadas de mundo
    float y;
    float velX;
    float velY;
    float lado;
    float alcanceRestante;
    int   dano;
    bool  ativo;

public:
    Projetil();
    // dirX/dirY nao precisam estar normalizados; o construtor normaliza.
    Projetil(float origemX, float origemY, float dirX, float dirY, int dano);

    void atualizar(float dt);

    bool get_ativo() const;
    void desativar();
    int  get_dano() const;

    float get_x() const;
    float get_y() const;
    SDL_FRect get_retangulo() const;

    void desenhar(SDL_Renderer*, const Camera&) const;
};

#endif
