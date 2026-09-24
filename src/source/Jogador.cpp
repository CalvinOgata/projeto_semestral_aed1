#include "Jogador.h"

#include <cmath>

#include "Config.h"

Jogador::Jogador()
    : vida(0), estamina(0), vidaMax(0), estaminaMax(0),
      x(0.0f), y(0.0f),
      largura(LADO_JOGADOR), altura(LADO_JOGADOR),
      velocidade(VELOCIDADE_JOGADOR)
{
    // mudar_vidaMax()/mudar_estaminaMax() somam ao valor atual, por isso os
    // atributos comecam zerados na lista de inicializacao acima.
    this->mudar_vidaMax(100);
    this->mudar_estaminaMax(100);
    this->rec_vida(this->get_vidaMax());
    this->rec_estamina(this->get_estaminaMax());
}

Jogador::~Jogador()
{
}

void Jogador::rec_vida(int quantidade)
{
    this->vida += quantidade;

    if(this->get_vida() > this->get_vidaMax())
    {
        this->vida = this->vidaMax;
    }
}

void Jogador::rec_estamina(int quantidade)
{
    this->estamina += quantidade;

    if(this->get_estamina() > this->get_estaminaMax())
    {
        this->estamina = this->estaminaMax;
    }
}

void Jogador::perder_vida(int quantidade)
{
    this->vida -= quantidade;

    if(this->vida < 0)
    {
        this->vida = 0;
    }
}

void Jogador::perder_estamina(int quantidade)
{
    this->estamina -= quantidade;

    if(this->estamina < 0)
    {
        this->estamina = 0;
    }
}

void Jogador::mudar_vidaMax(int quantidade)
{
    this->vidaMax += quantidade;

    if(this->get_vidaMax() < 1)
    {
        this->vidaMax = 1;
    }
    if(this->get_vida() > this->get_vidaMax())
    {
        this->vida = this->vidaMax;
    }
}

void Jogador::mudar_estaminaMax(int quantidade)
{
    this->estaminaMax += quantidade;

    if(this->get_estaminaMax() < 1)
    {
        this->estaminaMax = 1;
    }
    if(this->get_estamina() > this->get_estaminaMax())
    {
        this->estamina = this->estaminaMax;
    }
}

int Jogador::get_vida() const
{
    return this->vida;
}

int Jogador::get_estamina() const
{
    return this->estamina;
}

int Jogador::get_vidaMax() const
{
    return this->vidaMax;
}

int Jogador::get_estaminaMax() const
{
    return this->estaminaMax;
}

bool Jogador::Morto() const
{
    if(this->get_vida() > 0)
    {
        return false;
    }
    return true;
}

void Jogador::set_posicao(float novoX, float novoY)
{
    this->x = novoX;
    this->y = novoY;
}

void Jogador::set_velocidade(float nova)
{
    if(nova < 0.0f)
    {
        this->velocidade = 0.0f;
    }
    else
    {
        this->velocidade = nova;
    }
}

void Jogador::mover(int dirX, int dirY, float dt)
{
    if(dirX == 0 && dirY == 0)
    {
        return;
    }

    float dx = static_cast<float>(dirX);
    float dy = static_cast<float>(dirY);

    // Normaliza para a diagonal nao ficar mais rapida que a linha reta.
    const float comprimento = std::sqrt(dx*dx + dy*dy);
    dx /= comprimento;
    dy /= comprimento;

    this->x += dx * this->velocidade * dt;
    this->y += dy * this->velocidade * dt;
}

void Jogador::manter_nos_limites(const SDL_Rect &limites)
{
    const float minX = static_cast<float>(limites.x);
    const float minY = static_cast<float>(limites.y);
    const float maxX = static_cast<float>(limites.x + limites.w) - this->largura;
    const float maxY = static_cast<float>(limites.y + limites.h) - this->altura;

    if(this->x < minX) this->x = minX;
    if(this->y < minY) this->y = minY;
    if(this->x > maxX) this->x = maxX;
    if(this->y > maxY) this->y = maxY;
}

float Jogador::get_x() const
{
    return this->x;
}

float Jogador::get_y() const
{
    return this->y;
}

float Jogador::get_largura() const
{
    return this->largura;
}

float Jogador::get_altura() const
{
    return this->altura;
}

float Jogador::get_velocidade() const
{
    return this->velocidade;
}

SDL_FRect Jogador::get_retangulo() const
{
    SDL_FRect retangulo;
    retangulo.x = this->x;
    retangulo.y = this->y;
    retangulo.w = this->largura;
    retangulo.h = this->altura;
    return retangulo;
}

FilaArmas& Jogador::get_armas()
{
    return this->armas;
}

const FilaArmas& Jogador::get_armas() const
{
    return this->armas;
}

void Jogador::desenhar(SDL_Renderer *renderizador) const
{
    if(renderizador == nullptr)
    {
        return;
    }

    const SDL_FRect corpo = this->get_retangulo();

    SDL_SetRenderDrawColor(renderizador, COR_JOGADOR.r, COR_JOGADOR.g, COR_JOGADOR.b, COR_JOGADOR.a);
    SDL_RenderFillRectF(renderizador, &corpo);

    SDL_SetRenderDrawColor(renderizador, COR_JOGADOR_BORDA.r, COR_JOGADOR_BORDA.g, COR_JOGADOR_BORDA.b, COR_JOGADOR_BORDA.a);
    SDL_RenderDrawRectF(renderizador, &corpo);
}
