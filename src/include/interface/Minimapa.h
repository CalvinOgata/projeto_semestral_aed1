#ifndef MINIMAPA_H
#define MINIMAPA_H

#include "mundo/Fase.h"

namespace Minimapa
{
void desenhar(SDL_Renderer *renderizador, const Fase &fase, const Jogador &jogador);
}

#endif
