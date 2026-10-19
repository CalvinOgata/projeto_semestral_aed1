#ifndef AMBIENTE_H
#define AMBIENTE_H

#include <SDL2/SDL.h>

// Texturas geometricas deterministicas, sem arquivos externos.
namespace Ambiente
{
void desenhar_piso(SDL_Renderer *renderizador, const SDL_FRect &area);
void desenhar_paredes(SDL_Renderer *renderizador, const SDL_FRect &area);
void desenhar_porta(SDL_Renderer *renderizador, const SDL_FRect &area, bool aberta);
void desenhar_portal(SDL_Renderer *renderizador, float x, float y);
}

#endif
