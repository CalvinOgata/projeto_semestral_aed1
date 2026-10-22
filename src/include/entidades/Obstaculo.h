#ifndef OBSTACULO_H
#define OBSTACULO_H

#include <SDL2/SDL.h>

#include "graficos/Camera.h"

// Caixote: pacote de dados de madeira. Barreira: um bloquinho de firewall; as
// barreiras das salas sao fileiras desses blocos, que quebram um de cada vez.
enum class TipoObstaculo { CAIXOTE, BARREIRA };

// Cobertura dentro da sala. Bloqueia corpos e tiros; so ataques do jogador causam dano.
class Obstaculo
{
private:
    TipoObstaculo tipo;
    SDL_FRect area; // em coordenadas locais da sala
    int vida;
    int vidaMax;

public:
    Obstaculo(TipoObstaculo tipo, float centroX, float centroY);
    bool get_intacto() const;
    void levar_dano(int quantidade);
    SDL_FRect get_retangulo() const;
    // Menor deslocamento que tira o corpo de dentro do obstaculo; {0, 0} sem contato.
    SDL_FPoint separar(const SDL_FRect &corpo) const;
    void desenhar(SDL_Renderer *renderizador, const Camera &camera, bool mostrarHitbox) const;
};

#endif
