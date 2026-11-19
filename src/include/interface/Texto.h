#ifndef TEXTO_H
#define TEXTO_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

// Fonte de bitmap embutida (5x7). Evita depender do SDL2_ttf e de arquivos
// de fonte externos: cada caractere e desenhado com retangulos.
namespace Texto
{
constexpr int LARGURA_GLIFO = 5;
constexpr int ALTURA_GLIFO = 7;
constexpr int ESPACO_GLIFO = 1; // colunas em branco entre caracteres

int largura(const std::string &texto, int escala);
int altura(int escala);

void desenhar(SDL_Renderer *renderizador, const std::string &texto, int x, int y, int escala,
              SDL_Color cor);

// Quebra o texto em linhas que cabem na largura, sem partir palavras.
std::vector<std::string> quebrar_linhas(const std::string &texto, int larguraMaxima, int escala);

void desenhar_centralizado(SDL_Renderer *renderizador, const std::string &texto, int centroX, int y,
                           int escala, SDL_Color cor);
} // namespace Texto

#endif
