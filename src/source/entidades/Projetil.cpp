#include "entidades/Projetil.h"

#include <cmath>
#include <algorithm>

#include "config/Config.h"

Projetil::Projetil()
    : x(0.0f), y(0.0f), velX(0.0f), velY(0.0f), lado(LADO_PROJETIL), alcanceRestante(0.0f), dano(0),
      ativo(false)
{
}

Projetil::Projetil(float origemX, float origemY, float dirX, float dirY, int dano, TipoArma tipo)
    : x(origemX), y(origemY), velX(0.0f), velY(0.0f), lado(LADO_PROJETIL),
      alcanceRestante(ALCANCE_PROJETIL), dano(dano), ativo(true), tipo(tipo)
{
    const float comprimento = std::sqrt(dirX * dirX + dirY * dirY);

    if (comprimento <= 0.0f)
    {
        // Sem direcao nao ha tiro: nasce ja desativado.
        this->ativo = false;
        return;
    }

    if (tipo == TipoArma::BOOMERANG)
    {
        alcanceRestante = 260;
        lado = 18;
    }
    if (tipo == TipoArma::ARCO)
    {
        alcanceRestante = 1200;
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
    if (tipo == TipoArma::ARCO)
    {
        const float angulo = std::atan2(velY, velX);
        const float dx = std::cos(angulo), dy = std::sin(angulo);
        const auto centro = camera.para_tela(x, y);
        SDL_RenderDrawLineF(renderizador, centro.x - dx * 16, centro.y - dy * 16, centro.x + dx * 8, centro.y + dy * 8);
        SDL_RenderDrawLineF(renderizador, centro.x + dx * 8, centro.y + dy * 8, centro.x - dy * 5, centro.y + dx * 5);
        SDL_RenderDrawLineF(renderizador, centro.x + dx * 8, centro.y + dy * 8, centro.x + dy * 5, centro.y - dx * 5);
    }
    else if (tipo == TipoArma::BOOMERANG)
    {
        const float giro = alcanceRestante * 0.05f;
        const auto centro = camera.para_tela(x, y);
        SDL_RenderDrawLineF(renderizador, centro.x, centro.y, centro.x + std::cos(giro) * 14, centro.y + std::sin(giro) * 14);
        SDL_RenderDrawLineF(renderizador, centro.x, centro.y, centro.x - std::sin(giro) * 14, centro.y + std::cos(giro) * 14);
    }
    else
    {
        SDL_RenderFillRectF(renderizador, &naTela);
    }
}

void Projetil::atualizar(float dt, const SDL_FRect &limites, SDL_FPoint alvo)
{
    if (!ativo)
    {
        return;
    }
    if (tipo == TipoArma::BOOMERANG && retornando)
    {
        float dx = alvo.x - x, dy = alvo.y - y;
        const float distancia = std::hypot(dx, dy);
        if (distancia <= 20 + VELOCIDADE_PROJETIL * dt)
        {
            desativar();
            return;
        }
        velX = dx / distancia * VELOCIDADE_PROJETIL;
        velY = dy / distancia * VELOCIDADE_PROJETIL;
    }
    atualizar(dt);
    if (tipo == TipoArma::BOOMERANG && !retornando && !ativo)
    {
        ativo = true;
        retornando = true;
        alcanceRestante = 3000;
        atingidos.clear();
    }
    const float margem = lado / 2;
    const bool foraX = x < limites.x + margem || x > limites.x + limites.w - margem;
    const bool foraY = y < limites.y + margem || y > limites.y + limites.h - margem;
    if (!foraX && !foraY)
    {
        return;
    }
    if (tipo == TipoArma::RICOCHETE && ricochetes > 0)
    {
        if (foraX)
        {
            velX = -velX;
        }
        if (foraY)
        {
            velY = -velY;
        }
        --ricochetes;
    }
    else if (tipo == TipoArma::BOOMERANG)
    {
        if (!retornando)
        {
            retornando = true;
            alcanceRestante = 3000;
            atingidos.clear();
        }
    }
    else
    {
        desativar();
        return;
    }
    x = std::clamp(x, limites.x + margem, limites.x + limites.w - margem);
    y = std::clamp(y, limites.y + margem, limites.y + limites.h - margem);
}

bool Projetil::atingir(std::size_t indice)
{
    if (std::find(atingidos.begin(), atingidos.end(), indice) != atingidos.end())
    {
        return false;
    }
    if (tipo == TipoArma::BOOMERANG)
    {
        atingidos.push_back(indice);
    }
    else
    {
        desativar();
    }
    return true;
}

bool Projetil::colidir_obstaculo(const SDL_FRect &area)
{
    const SDL_FRect corpo = this->get_retangulo();
    if (!ativo || !SDL_HasIntersectionF(&corpo, &area))
    {
        return false;
    }
    if (tipo == TipoArma::BOOMERANG)
    {
        // Bate e volta, como na parede; na volta atravessa sem novo dano.
        if (retornando)
        {
            return false;
        }
        retornando = true;
        alcanceRestante = 3000;
        atingidos.clear();
        return true;
    }
    if (tipo == TipoArma::RICOCHETE && ricochetes > 0)
    {
        const float sobreposicaoX = std::min(corpo.x + corpo.w - area.x, area.x + area.w - corpo.x);
        const float sobreposicaoY = std::min(corpo.y + corpo.h - area.y, area.y + area.h - corpo.y);
        if (sobreposicaoX < sobreposicaoY)
        {
            velX = -velX;
            x += x < area.x + area.w / 2 ? -sobreposicaoX : sobreposicaoX;
        }
        else
        {
            velY = -velY;
            y += y < area.y + area.h / 2 ? -sobreposicaoY : sobreposicaoY;
        }
        --ricochetes;
        return true;
    }
    desativar();
    return true;
}
