#ifndef SPRITE_INIMIGO_H
#define SPRITE_INIMIGO_H

#include <SDL2/SDL.h>

#include "entidades/Inimigo.h"

// Bitmaps embutidos dos inimigos. Guardioes e boss variam com o tema da fase.
namespace SpriteInimigo
{
void desenhar(SDL_Renderer *renderizador, TipoInimigo tipo, int tema, const SDL_FRect &area);
}

#endif
