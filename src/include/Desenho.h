#ifndef DESENHO_H
#define DESENHO_H

#include <SDL2/SDL.h>

// Auxiliares de desenho que o SDL nao oferece direto com retangulos.
// Guarda uma textura de 1x1 pixel branco: esticada e girada, ela vira
// qualquer retangulo inclinado (o cano da arma, por exemplo).
namespace Desenho
{
    bool inicializar(SDL_Renderer*);
    void finalizar();

    // Desenha um retangulo girado em torno do meio da sua borda esquerda.
    void retangulo_rotacionado(SDL_Renderer *renderizador,
                               float pivoX, float pivoY,
                               float comprimento, float espessura,
                               float anguloGraus, SDL_Color cor);
}

#endif
