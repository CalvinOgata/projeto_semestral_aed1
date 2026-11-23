#ifndef CATALOGO_SPRITES_H
#define CATALOGO_SPRITES_H
#include "entidades/Armas.h"
#include "entidades/Inimigo.h"
// Recursos SDL carregados uma vez e liberados por Jogo antes do renderizador.
namespace CatalogoSprites
{
bool carregar(SDL_Renderer *);
void liberar();
bool desenhar_inimigo(SDL_Renderer *, TipoInimigo, Afinidade, const SDL_FRect &, float tempo);
bool desenhar_arma(SDL_Renderer *, TipoArma, const SDL_FRect &);
bool desenhar_empunhada(SDL_Renderer *, TipoArma, SDL_FPoint mao, float angulo);
bool desenhar_piso(SDL_Renderer *, int indice, const SDL_FRect &);
} // namespace CatalogoSprites
#endif
