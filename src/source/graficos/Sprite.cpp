#include "graficos/Sprite.h"

#include <png.h>

#include <algorithm>
#include <vector>

Sprite::~Sprite()
{
    this->liberar();
}

bool Sprite::carregar(SDL_Renderer *renderizador, const std::string &caminho)
{
    this->liberar();
    this->erro.clear();
    if (renderizador == nullptr)
    {
        this->erro = "Renderizador ausente";
        return false;
    }
    png_image imagem{};
    imagem.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&imagem, caminho.c_str()))
    {
        this->erro = imagem.message;
        png_image_free(&imagem);
        return false;
    }
    // Limite de recurso para evitar alocacoes excessivas ao trocar um asset.
    if (imagem.width == 0 || imagem.height == 0 || imagem.width > 4096 || imagem.height > 4096)
    {
        this->erro = "Dimensoes de PNG invalidas ou maiores que 4096 pixels";
        png_image_free(&imagem);
        return false;
    }
    imagem.format = PNG_FORMAT_RGBA;
    std::vector<png_byte> pixels(PNG_IMAGE_SIZE(imagem));
    if (!png_image_finish_read(&imagem, nullptr, pixels.data(), 0, nullptr))
    {
        this->erro = imagem.message;
        png_image_free(&imagem);
        return false;
    }
    this->largura = static_cast<int>(imagem.width);
    this->altura = static_cast<int>(imagem.height);
    png_image_free(&imagem);

    SDL_Surface *superficie = SDL_CreateRGBSurfaceWithFormatFrom(
        pixels.data(), this->largura, this->altura, 32, this->largura * 4, SDL_PIXELFORMAT_RGBA32);
    if (superficie == nullptr)
    {
        this->erro = SDL_GetError();
        return false;
    }
    this->textura = SDL_CreateTextureFromSurface(renderizador, superficie);
    SDL_FreeSurface(superficie);
    if (this->textura == nullptr)
    {
        this->erro = SDL_GetError();
        return false;
    }
    if (SDL_SetTextureBlendMode(this->textura, SDL_BLENDMODE_BLEND) != 0 ||
        SDL_SetTextureScaleMode(this->textura, SDL_ScaleModeNearest) != 0)
    {
        this->erro = SDL_GetError();
        this->liberar();
        return false;
    }
    return true;
}

void Sprite::liberar()
{
    if (this->textura != nullptr)
    {
        SDL_DestroyTexture(this->textura);
        this->textura = nullptr;
    }
    this->largura = 0;
    this->altura = 0;
}

const std::string &Sprite::get_erro() const
{
    return this->erro;
}

bool Sprite::desenhar(SDL_Renderer *renderizador, const SDL_FRect &quadro) const
{
    if (renderizador == nullptr || this->textura == nullptr || quadro.w <= 0 || quadro.h <= 0)
    {
        return false;
    }
    // Encaixa a imagem no quadro, centralizada, sem esticar ou recortar.
    const float escala = std::min(quadro.w / this->largura, quadro.h / this->altura);
    const float w = this->largura * escala;
    const float h = this->altura * escala;
    const SDL_FRect destino = {quadro.x + (quadro.w - w) / 2.0f,
                              quadro.y + (quadro.h - h) / 2.0f, w, h};
    return SDL_RenderCopyF(renderizador, this->textura, nullptr, &destino) == 0;
}
