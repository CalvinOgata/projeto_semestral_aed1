#ifndef FUNDO_MENU_H
#define FUNDO_MENU_H

#include <SDL2/SDL.h>

// Cenario animado das telas fora da partida: ceu com chuva binaria, sol de
// circuitos e grade em perspectiva. Tudo procedural, guiado so pelo tempo.
namespace FundoMenu
{
// Altura da linha do horizonte, onde a grade comeca.
constexpr int HORIZONTE = 330;

void desenhar(SDL_Renderer *renderizador, float tempo, float intensidade = 1.0f);

// Logotipo "M.C.G.A" com sombra e falhas de cromatismo ocasionais.
void desenhar_logo(SDL_Renderer *renderizador, int centroX, int y, int escala, float tempo,
                   Uint8 alfa = 255);

// Painel translucido de vidro com cantos destacados.
void desenhar_painel(SDL_Renderer *renderizador, const SDL_Rect &area, float tempo);

// Numero pseudoaleatorio estavel (0 a 1) a partir de uma semente inteira.
float sorteio(int semente);

// Retangulo preenchido com transparencia, usado por varios efeitos.
void preencher(SDL_Renderer *renderizador, const SDL_Rect &area, SDL_Color cor);
} // namespace FundoMenu

#endif
