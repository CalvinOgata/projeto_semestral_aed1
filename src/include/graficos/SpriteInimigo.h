#ifndef SPRITE_INIMIGO_H
#define SPRITE_INIMIGO_H

#include <SDL2/SDL.h>

#include "entidades/Inimigo.h"

// Bitmaps embutidos temporarios das nove combinacoes de classe e afinidade.
namespace SpriteInimigo
{
void desenhar(SDL_Renderer *renderizador, TipoInimigo tipo, Afinidade afinidade,
              const SDL_FRect &area);
}

#endif
