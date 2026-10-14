#include "Camera.h"

#include "Config.h"

Camera::Camera() : x(0.0f), y(0.0f)
{
}

void Camera::centralizar_em(float mundoX, float mundoY)
{
    this->x = mundoX - LARGURA_TELA / 2.0f;
    this->y = mundoY - ALTURA_TELA / 2.0f;
}

float Camera::get_x() const
{
    return this->x;
}

float Camera::get_y() const
{
    return this->y;
}

SDL_FPoint Camera::para_tela(float mundoX, float mundoY) const
{
    SDL_FPoint ponto;
    ponto.x = mundoX - this->x;
    ponto.y = mundoY - this->y;
    return ponto;
}

SDL_FRect Camera::para_tela(const SDL_FRect &mundo) const
{
    SDL_FRect naTela;
    naTela.x = mundo.x - this->x;
    naTela.y = mundo.y - this->y;
    naTela.w = mundo.w;
    naTela.h = mundo.h;
    return naTela;
}

SDL_FPoint Camera::para_mundo(float telaX, float telaY) const
{
    SDL_FPoint ponto;
    ponto.x = telaX + this->x;
    ponto.y = telaY + this->y;
    return ponto;
}
