#include "entidades/Inimigo.h"

#include <algorithm>
#include <cmath>

#include "config/Config.h"

Inimigo::Inimigo()
    : x(0.0f), y(0.0f), lado(LADO_INIMIGO), velocidade(VELOCIDADE_INIMIGO), vida(VIDA_INIMIGO),
      vivo(true)
{
}

Inimigo::Inimigo(float mundoX, float mundoY)
    : x(mundoX), y(mundoY), lado(LADO_INIMIGO), velocidade(VELOCIDADE_INIMIGO), vida(VIDA_INIMIGO),
      vivo(true)
{
}

void Inimigo::perseguir(float alvoX, float alvoY, float dt)
{
    if (!this->vivo)
    {
        return;
    }

    float dx = alvoX - this->get_centroX();
    float dy = alvoY - this->get_centroY();

    const float distancia = std::sqrt(dx * dx + dy * dy);

    if (distancia <= 0.001f)
    {
        return; // ja esta em cima do alvo
    }

    dx /= distancia;
    dy /= distancia;

    this->x += dx * this->velocidade * dt;
    this->y += dy * this->velocidade * dt;
}

void Inimigo::afastar_de(Inimigo &outro)
{
    if (!this->vivo || !outro.vivo)
    {
        return;
    }

    float dx = this->get_centroX() - outro.get_centroX();
    float dy = this->get_centroY() - outro.get_centroY();

    const float minimo = this->lado;
    float distancia = std::sqrt(dx * dx + dy * dy);

    if (distancia >= minimo)
    {
        return; // nao estao se sobrepondo
    }

    if (distancia <= 0.001f)
    {
        // Centros coincidentes: separa numa direcao qualquer para nao dividir por zero.
        dx = 1.0f;
        dy = 0.0f;
        distancia = 1.0f;
    }

    // Cada um anda metade da sobreposicao, em sentidos opostos.
    const float empurrao = (minimo - distancia) / 2.0f;
    const float passoX = (dx / distancia) * empurrao;
    const float passoY = (dy / distancia) * empurrao;

    this->x += passoX;
    this->y += passoY;
    outro.x -= passoX;
    outro.y -= passoY;
}

void Inimigo::levar_dano(int quantidade)
{
    if (!this->vivo || quantidade <= 0)
    {
        return;
    }

    this->vida -= quantidade;

    if (this->vida <= 0)
    {
        this->vida = 0;
        this->vivo = false;
    }
}

bool Inimigo::get_vivo() const
{
    return this->vivo;
}

int Inimigo::get_vida() const
{
    return this->vida;
}

float Inimigo::get_centroX() const
{
    return this->x + this->lado / 2.0f;
}

float Inimigo::get_centroY() const
{
    return this->y + this->lado / 2.0f;
}

float Inimigo::get_lado() const
{
    return this->lado;
}

SDL_FRect Inimigo::get_retangulo() const
{
    SDL_FRect retangulo;
    retangulo.x = this->x;
    retangulo.y = this->y;
    retangulo.w = this->lado;
    retangulo.h = this->lado;
    return retangulo;
}

void Inimigo::desenhar(SDL_Renderer *renderizador, const Camera &camera) const
{
    if (renderizador == nullptr || !this->vivo)
    {
        return;
    }

    const SDL_FRect naTela = camera.para_tela(this->get_retangulo());

    SDL_SetRenderDrawColor(renderizador, COR_INIMIGO.r, COR_INIMIGO.g, COR_INIMIGO.b,
                           COR_INIMIGO.a);
    SDL_RenderFillRectF(renderizador, &naTela);

    SDL_SetRenderDrawColor(renderizador, COR_INIMIGO_BORDA.r, COR_INIMIGO_BORDA.g,
                           COR_INIMIGO_BORDA.b, COR_INIMIGO_BORDA.a);
    SDL_RenderDrawRectF(renderizador, &naTela);
}

Inimigo::Inimigo(float mundoX, float mundoY, int vidaInicial, float velocidadeInicial)
    : x(mundoX), y(mundoY), lado(LADO_INIMIGO), velocidade(std::max(0.0f, velocidadeInicial)),
      vida(std::max(1, vidaInicial)), vivo(true)
{
}

void Inimigo::limitar_a(const SDL_FRect &area)
{
    this->x = std::clamp(this->x, area.x, area.x + area.w - this->lado);
    this->y = std::clamp(this->y, area.y, area.y + area.h - this->lado);
}
