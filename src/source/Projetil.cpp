#include "Projetil.h"

#include <cmath>

#include "Config.h"

Projetil::Projetil()
    : x(0.0f), y(0.0f), velX(0.0f), velY(0.0f), lado(LADO_PROJETIL), alcanceRestante(0.0f), dano(0),
      ativo(false)
{
}

Projetil::Projetil(float origemX, float origemY, float dirX, float dirY, int dano)
    : x(origemX), y(origemY), velX(0.0f), velY(0.0f), lado(LADO_PROJETIL),
      alcanceRestante(ALCANCE_PROJETIL), dano(dano), ativo(true)
{
    const float comprimento = std::sqrt(dirX * dirX + dirY * dirY);

    if (comprimento <= 0.0f)
    {
        // Sem direcao nao ha tiro: nasce ja desativado.
        this->ativo = false;
        return;
    }

    this->velX = (dirX / comprimento) * VELOCIDADE_PROJETIL;
    this->velY = (dirY / comprimento) * VELOCIDADE_PROJETIL;
}

void Projetil::atualizar(float dt)
{
    if (!this->ativo)
    {
        return;
    }

    const float passoX = this->velX * dt;
    const float passoY = this->velY * dt;

    this->x += passoX;
    this->y += passoY;

    this->alcanceRestante -= std::sqrt(passoX * passoX + passoY * passoY);

    if (this->alcanceRestante <= 0.0f)
    {
        this->ativo = false;
    }
}

bool Projetil::get_ativo() const
{
    return this->ativo;
}

void Projetil::desativar()
{
    this->ativo = false;
}

int Projetil::get_dano() const
{
    return this->dano;
}

float Projetil::get_x() const
{
    return this->x;
}

float Projetil::get_y() const
{
    return this->y;
}

SDL_FRect Projetil::get_retangulo() const
{
    SDL_FRect retangulo;
    retangulo.x = this->x - this->lado / 2.0f;
    retangulo.y = this->y - this->lado / 2.0f;
    retangulo.w = this->lado;
    retangulo.h = this->lado;
    return retangulo;
}

void Projetil::desenhar(SDL_Renderer *renderizador, const Camera &camera) const
{
    if (renderizador == nullptr || !this->ativo)
    {
        return;
    }

    const SDL_FRect naTela = camera.para_tela(this->get_retangulo());

    SDL_SetRenderDrawColor(renderizador, COR_PROJETIL.r, COR_PROJETIL.g, COR_PROJETIL.b,
                           COR_PROJETIL.a);
    SDL_RenderFillRectF(renderizador, &naTela);
}
