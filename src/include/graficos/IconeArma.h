#ifndef ICONE_ARMA_H
#define ICONE_ARMA_H

#include <SDL2/SDL.h>

#include "entidades/Armas.h"

// Sprites de bitmap embutidos: nao precisam de arquivos ou texturas externas.
namespace IconeArma
{
void desenhar(SDL_Renderer *renderizador, TipoArma tipo, const SDL_FRect &area);
void desenhar_empunhada(SDL_Renderer *renderizador, TipoArma tipo, SDL_FPoint mao,
                       float anguloGraus);
}

#endif
