#include "graficos/Ambiente.h"

#include <algorithm>
#include <cmath>

namespace
{
void bloco(SDL_Renderer *r, float x, float y, float w, float h, SDL_Color cor)
{
    SDL_SetRenderDrawColor(r, cor.r, cor.g, cor.b, cor.a);
    const SDL_FRect area{x, y, w, h};
    SDL_RenderFillRectF(r, &area);
}
}

void Ambiente::desenhar_piso(SDL_Renderer *r, const SDL_FRect &area)
{
    bloco(r, area.x, area.y, area.w, area.h, {25, 29, 35, 255});
    for (int linha = 0; linha * 48 < area.h; ++linha)
        for (int coluna = 0; coluna * 48 < area.w; ++coluna)
        {
            const float x = area.x + coluna * 48, y = area.y + linha * 48;
            const float w = std::min(46.0f, area.w - coluna * 48 - 2);
            const float h = std::min(46.0f, area.h - linha * 48 - 2);
            if (w <= 0 || h <= 0) continue;
            const Uint8 tom = 43 + (coluna * 17 + linha * 31) % 11;
            bloco(r, x + 1, y + 1, w, h, {tom, Uint8(tom + 6), Uint8(tom + 12), 255});
            bloco(r, x + 2, y + 2, w - 2, 1, {65, 73, 82, 255});
            if (w > 30 && h > 30 && (coluna + linha * 3) % 7 == 0)
            {
                bloco(r, x + 8, y + 12, 18, 2, {38, 65, 70, 255});
                bloco(r, x + 24, y + 12, 2, 18, {38, 65, 70, 255});
                bloco(r, x + 22, y + 28, 6, 4, {55, 92, 92, 255});
            }
            if (w > 12 && h > 12)
                bloco(r, x + w - 5, y + h - 5, 2, 2, {29, 34, 42, 255});
        }
}

void Ambiente::desenhar_paredes(SDL_Renderer *r, const SDL_FRect &a)
{
    bloco(r, a.x - 12, a.y - 12, a.w + 24, 12, {80, 89, 101, 255});
    bloco(r, a.x - 12, a.y + a.h, a.w + 24, 12, {44, 50, 63, 255});
    bloco(r, a.x - 12, a.y, 12, a.h, {60, 69, 82, 255});
    bloco(r, a.x + a.w, a.y, 12, a.h, {44, 50, 63, 255});
    for (float x = a.x; x < a.x + a.w; x += 48)
    {
        bloco(r, x, a.y - 12, 2, 12, {25, 30, 39, 255});
        bloco(r, x, a.y + a.h, 2, 12, {25, 30, 39, 255});
    }
    for (float y = a.y; y < a.y + a.h; y += 48)
    {
        bloco(r, a.x - 12, y, 12, 2, {25, 30, 39, 255});
        bloco(r, a.x + a.w, y, 12, 2, {25, 30, 39, 255});
    }
    bloco(r, a.x, a.y, a.w, 5, {23, 28, 36, 255});
    bloco(r, a.x, a.y, 4, a.h, {23, 28, 36, 255});
}

void Ambiente::desenhar_porta(SDL_Renderer *r, const SDL_FRect &a, bool aberta)
{
    const bool vertical = a.h > a.w;
    bloco(r, a.x, a.y, a.w, a.h, {24, 29, 37, 255});
    const SDL_Color luz = aberta ? SDL_Color{75, 186, 173, 255} : SDL_Color{210, 77, 65, 255};
    if (vertical)
    {
        bloco(r, a.x, a.y, a.w, 6, luz);
        bloco(r, a.x, a.y + a.h - 6, a.w, 6, luz);
        if (!aberta)
            for (float y = a.y + 10; y < a.y + a.h - 6; y += 12)
                bloco(r, a.x + 3, y, a.w - 6, 5, {134, 84, 69, 255});
    }
    else
    {
        bloco(r, a.x, a.y, 6, a.h, luz);
        bloco(r, a.x + a.w - 6, a.y, 6, a.h, luz);
        if (!aberta)
            for (float x = a.x + 10; x < a.x + a.w - 6; x += 12)
                bloco(r, x, a.y + 3, 5, a.h - 6, {134, 84, 69, 255});
    }
}

void Ambiente::desenhar_portal(SDL_Renderer *r, float x, float y)
{
    // Boca escura no chao, borda de pedra irregular e aneis violetas.
    for (int raio = 54; raio > 0; --raio)
    {
        const SDL_Color cor = raio > 46 ? SDL_Color{66, 61, 79, 255} :
                              raio > 39 ? SDL_Color{132, 103, 170, 255} :
                              raio > 32 ? SDL_Color{62, 43, 93, 255} :
                                          SDL_Color{12, 10, 23, 255};
        SDL_SetRenderDrawColor(r, cor.r, cor.g, cor.b, 255);
        for (int a = 0; a < 180; ++a)
        {
            const float angulo = a * 6.2831853f / 180;
            SDL_RenderDrawPointF(r, x + std::cos(angulo) * raio,
                                y + std::sin(angulo) * raio * 0.65f);
        }
    }
    for (int i = 0; i < 10; ++i)
    {
        const float a = i * 6.2831853f / 10;
        bloco(r, x + std::cos(a) * 49 - 2, y + std::sin(a) * 32 - 2,
              4, 4, {207, 176, 231, 255});
    }
}
