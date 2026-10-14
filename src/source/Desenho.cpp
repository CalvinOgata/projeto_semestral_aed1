#include "Desenho.h"

namespace
{
SDL_Texture *pixelBranco = nullptr;
}

bool Desenho::inicializar(SDL_Renderer *renderizador)
{
    if (renderizador == nullptr)
    {
        return false;
    }

    Desenho::finalizar();

    pixelBranco =
        SDL_CreateTexture(renderizador, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
    if (pixelBranco == nullptr)
    {
        return false;
    }

    const Uint32 branco = 0xFFFFFFFFu;
    SDL_UpdateTexture(pixelBranco, nullptr, &branco, sizeof(branco));
    SDL_SetTextureBlendMode(pixelBranco, SDL_BLENDMODE_BLEND);
    return true;
}

void Desenho::finalizar()
{
    if (pixelBranco != nullptr)
    {
        SDL_DestroyTexture(pixelBranco);
        pixelBranco = nullptr;
    }
}

void Desenho::retangulo_rotacionado(SDL_Renderer *renderizador, float pivoX, float pivoY,
                                    float comprimento, float espessura, float anguloGraus,
                                    SDL_Color cor)
{
    if (renderizador == nullptr || pixelBranco == nullptr)
    {
        return;
    }

    SDL_FRect destino;
    destino.x = pivoX;
    destino.y = pivoY - espessura / 2.0f;
    destino.w = comprimento;
    destino.h = espessura;

    // Gira em torno do meio da borda esquerda, nao do centro do retangulo.
    const SDL_FPoint pivo = {0.0f, espessura / 2.0f};

    SDL_SetTextureColorMod(pixelBranco, cor.r, cor.g, cor.b);
    SDL_SetTextureAlphaMod(pixelBranco, cor.a);
    SDL_RenderCopyExF(renderizador, pixelBranco, nullptr, &destino, anguloGraus, &pivo,
                      SDL_FLIP_NONE);
}
