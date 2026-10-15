#include "entidades/Inimigo.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "config/Config.h"

Inimigo::Inimigo() : Inimigo(0.0f, 0.0f)
{
}

Inimigo::Inimigo(float mundoX, float mundoY)
    : Inimigo(mundoX, mundoY, VIDA_INIMIGO, VELOCIDADE_INIMIGO)
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

    const float dx = this->get_centroX() - outro.get_centroX();
    const float dy = this->get_centroY() - outro.get_centroY();
    const float sobreposicaoX = (this->get_largura() + outro.get_largura()) / 2.0f - std::abs(dx);
    const float sobreposicaoY = (this->get_altura() + outro.get_altura()) / 2.0f - std::abs(dy);
    if (sobreposicaoX <= 0.0f || sobreposicaoY <= 0.0f)
    {
        return;
    }
    // Separa pelo menor eixo, respeitando hitboxes retangulares de tamanhos diferentes.
    if (sobreposicaoX <= sobreposicaoY)
    {
        const float passo = (dx >= 0.0f ? 1.0f : -1.0f) * sobreposicaoX / 2.0f;
        this->x += passo;
        outro.x -= passo;
    }
    else
    {
        const float passo = (dy >= 0.0f ? 1.0f : -1.0f) * sobreposicaoY / 2.0f;
        this->y += passo;
        outro.y -= passo;
    }
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
    return this->x + this->get_largura() / 2.0f;
}

float Inimigo::get_centroY() const
{
    return this->y + this->get_altura() / 2.0f;
}

float Inimigo::get_largura() const
{
    return static_cast<float>(this->dimensoes.larguraHitbox);
}

float Inimigo::get_altura() const
{
    return static_cast<float>(this->dimensoes.alturaHitbox);
}

SDL_FRect Inimigo::get_retangulo() const
{
    SDL_FRect retangulo;
    retangulo.x = this->x;
    retangulo.y = this->y;
    retangulo.w = this->get_largura();
    retangulo.h = this->get_altura();
    return retangulo;
}

void Inimigo::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool mostrarHitbox) const
{
    if (renderizador == nullptr || !this->vivo)
    {
        return;
    }

    const SDL_FRect naTela = camera.para_tela(this->get_retanguloVisual());

    SDL_SetRenderDrawColor(renderizador, COR_INIMIGO.r, COR_INIMIGO.g, COR_INIMIGO.b,
                           COR_INIMIGO.a);
    SDL_RenderFillRectF(renderizador, &naTela);

    SDL_SetRenderDrawColor(renderizador, COR_INIMIGO_BORDA.r, COR_INIMIGO_BORDA.g,
                           COR_INIMIGO_BORDA.b, COR_INIMIGO_BORDA.a);
    SDL_RenderDrawRectF(renderizador, &naTela);

    // Barra simples acima do corpo, proporcional a vida inicial deste inimigo.
    const SDL_FRect fundo = {naTela.x, naTela.y - 8.0f, naTela.w, 4.0f};
    SDL_SetRenderDrawColor(renderizador, 62, 24, 28, 255);
    SDL_RenderFillRectF(renderizador, &fundo);

    const float proporcao = static_cast<float>(this->vida) / this->vidaMax;
    const SDL_FRect barra = {fundo.x, fundo.y, fundo.w * proporcao, fundo.h};
    SDL_SetRenderDrawColor(renderizador, 230, 60, 70, 255);
    SDL_RenderFillRectF(renderizador, &barra);
    if (mostrarHitbox)
    {
        const SDL_FRect hitbox = camera.para_tela(this->get_retangulo());
        SDL_SetRenderDrawColor(renderizador, 100, 255, 140, 255);
        SDL_RenderDrawRectF(renderizador, &hitbox);
    }

}

Inimigo::Inimigo(float mundoX, float mundoY, int vidaInicial, float velocidadeInicial,
                 const DimensoesEntidade &dimensoes)
    : x(mundoX), y(mundoY), dimensoes(dimensoes), velocidade(std::max(0.0f, velocidadeInicial)),
      vida(std::max(1, vidaInicial)), vidaMax(std::max(1, vidaInicial)), vivo(true)
{
    if (!dimensoes.valida())
    {
        throw std::invalid_argument("Dimensoes de inimigo invalidas");
    }
}

SDL_FRect Inimigo::get_retanguloVisual() const
{
    const float largura = static_cast<float>(this->dimensoes.larguraSprite);
    const float altura = static_cast<float>(this->dimensoes.alturaSprite);
    return {this->get_centroX() - largura / 2.0f, this->get_centroY() - altura / 2.0f,
            largura, altura};
}

void Inimigo::limitar_a(const SDL_FRect &area)
{
    this->x = std::clamp(this->x, area.x, std::max(area.x, area.x + area.w - this->get_largura()));
    this->y = std::clamp(this->y, area.y, std::max(area.y, area.y + area.h - this->get_altura()));
}
