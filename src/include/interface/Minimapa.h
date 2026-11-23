#ifndef MINIMAPA_H
#define MINIMAPA_H

#include "mundo/Fase.h"

namespace Minimapa
{
// toque troca o aviso de tecla para fechar por um aviso de toque.
void desenhar(SDL_Renderer *renderizador, const Fase &fase, const Jogador &jogador,
              bool toque = false);
} // namespace Minimapa

#endif
