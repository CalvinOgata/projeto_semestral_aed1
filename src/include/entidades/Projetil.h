#ifndef PROJETIL_H
#define PROJETIL_H

#include <SDL2/SDL.h>
#include <vector>

#include "entidades/Armas.h"
#include "graficos/Camera.h"

class Projetil
{
private:
    float x; // centro, em coordenadas de mundo
    float y;
    float velX;
    float velY;
    float lado;
    float alcanceRestante;
    int dano;
    bool ativo;
    TipoArma tipo = TipoArma::PISTOLA;
    bool retornando = false;
    int ricochetes = 4;
    std::vector<std::size_t> atingidos;

public:
    Projetil();
    // dirX/dirY nao precisam estar normalizados; o construtor normaliza.
    Projetil(float origemX, float origemY, float dirX, float dirY, int dano,
             TipoArma tipo = TipoArma::PISTOLA);

    void atualizar(float dt);
    void atualizar(float dt, const SDL_FRect &limites, SDL_FPoint alvo);
    bool atingir(std::size_t indice);

    bool get_ativo() const;
    void desativar();
    int get_dano() const;

    float get_x() const;
    float get_y() const;
    SDL_FRect get_retangulo() const;

    void desenhar(SDL_Renderer *, const Camera &) const;
};

#endif
