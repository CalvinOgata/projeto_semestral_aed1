#ifndef HUD_H
#define HUD_H

#include "entidades/Jogador.h"

// Sobreposicao de leitura: nao recebe eventos nem reserva area da sala.
namespace Hud
{
void desenhar(SDL_Renderer *renderizador, const Jogador &jogador);
}

#endif
