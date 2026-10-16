#include "entidades/Jogador.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "config/Config.h"
#include "graficos/IconeArma.h"
#include "graficos/Sprite.h"

Jogador::Jogador(const DimensoesEntidade &dimensoes)
    : vida(0), escudo(50), escudoMax(50), estamina(0), vidaMax(0), estaminaMax(0),
      x(0.0f), y(0.0f), dimensoes(dimensoes), velocidade(VELOCIDADE_JOGADOR), anguloMira(0.0f)
{
    if (!dimensoes.valida())
    {
        throw std::invalid_argument("Dimensoes de jogador invalidas");
    }
    // mudar_vidaMax()/mudar_estaminaMax() somam ao valor atual, por isso os
    // atributos comecam zerados na lista de inicializacao acima.
    this->mudar_vidaMax(100);
    this->mudar_estaminaMax(100);
    this->rec_vida(this->get_vidaMax());
    this->rec_estamina(this->get_estaminaMax());

    // Arma inicial: entra na fila, que e de onde o jogo le o dano e a municao.
    Armas inicial;
    inicial.set_municao(MUNICAO_INICIAL);
    this->armas.inserir(inicial);
}

Jogador::~Jogador()
{
}

void Jogador::rec_vida(int quantidade)
{
    this->vida += quantidade;

    if (this->get_vida() > this->get_vidaMax())
    {
        this->vida = this->vidaMax;
    }
}

void Jogador::rec_estamina(int quantidade)
{
    this->estamina += quantidade;

    if (this->get_estamina() > this->get_estaminaMax())
    {
        this->estamina = this->estaminaMax;
    }
}

void Jogador::perder_vida(int quantidade)
{
    this->vida -= quantidade;

    if (this->vida < 0)
    {
        this->vida = 0;
    }
}

void Jogador::perder_estamina(int quantidade)
{
    this->estamina -= quantidade;

    if (this->estamina < 0)
    {
        this->estamina = 0;
    }
}

void Jogador::mudar_vidaMax(int quantidade)
{
    this->vidaMax += quantidade;

    if (this->get_vidaMax() < 1)
    {
        this->vidaMax = 1;
    }
    if (this->get_vida() > this->get_vidaMax())
    {
        this->vida = this->vidaMax;
    }
}

void Jogador::mudar_estaminaMax(int quantidade)
{
    this->estaminaMax += quantidade;

    if (this->get_estaminaMax() < 1)
    {
        this->estaminaMax = 1;
    }
    if (this->get_estamina() > this->get_estaminaMax())
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

bool Jogador::morto() const
{
    if (this->get_vida() > 0)
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
    if (nova < 0.0f)
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
    if (dirX == 0 && dirY == 0)
    {
        return;
    }

    float dx = static_cast<float>(dirX);
    float dy = static_cast<float>(dirY);

    // Normaliza para a diagonal nao ficar mais rapida que a linha reta.
    const float comprimento = std::sqrt(dx * dx + dy * dy);
    dx /= comprimento;
    dy /= comprimento;

    this->x += dx * this->velocidade * dt;
    this->y += dy * this->velocidade * dt;
}

float Jogador::get_x() const
{
    return this->x;
}

float Jogador::get_y() const
{
    return this->y;
}

float Jogador::get_centroX() const
{
    return this->x + this->dimensoes.larguraHitbox / 2.0f;
}

float Jogador::get_centroY() const
{
    return this->y + this->dimensoes.alturaHitbox / 2.0f;
}

float Jogador::get_largura() const
{
    return this->dimensoes.larguraHitbox;
}

float Jogador::get_altura() const
{
    return this->dimensoes.alturaHitbox;
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
    retangulo.w = this->dimensoes.larguraHitbox;
    retangulo.h = this->dimensoes.alturaHitbox;
    return retangulo;
}

SDL_FRect Jogador::get_retanguloVisual() const
{
    const float largura = static_cast<float>(this->dimensoes.larguraSprite);
    const float altura = static_cast<float>(this->dimensoes.alturaSprite);
    return {this->get_centroX() - largura / 2.0f, this->get_centroY() - altura / 2.0f,
            largura, altura};
}

void Jogador::mirar_em(float mundoX, float mundoY)
{
    const float dx = mundoX - this->get_centroX();
    const float dy = mundoY - this->get_centroY();

    if (dx == 0.0f && dy == 0.0f)
    {
        return; // mouse exatamente em cima: mantem a mira anterior
    }

    // atan2 devolve radianos; o SDL gira texturas em graus.
    this->anguloMira = std::atan2(dy, dx) * 180.0f / static_cast<float>(M_PI);
}

float Jogador::get_anguloMira() const
{
    return this->anguloMira;
}

SDL_FPoint Jogador::get_direcaoMira() const
{
    const float radianos = this->anguloMira * static_cast<float>(M_PI) / 180.0f;

    SDL_FPoint direcao;
    direcao.x = std::cos(radianos);
    direcao.y = std::sin(radianos);
    return direcao;
}

SDL_FPoint Jogador::get_bocaArma() const
{
    const SDL_FPoint direcao = this->get_direcaoMira();
    const float distancia = COMPRIMENTO_ARMA - RECUO_ARMA;

    SDL_FPoint boca;
    boca.x = this->get_centroX() + direcao.x * distancia;
    boca.y = this->get_centroY() + direcao.y * distancia;
    return boca;
}

Armas *Jogador::get_armaEquipada()
{
    return this->armas.frente();
}

FilaArmas &Jogador::get_armas()
{
    return this->armas;
}

const FilaArmas &Jogador::get_armas() const
{
    return this->armas;
}

void Jogador::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool mostrarHitbox,
                       const Sprite *sprite) const
{
    if (renderizador == nullptr)
    {
        return;
    }

    const SDL_FRect corpo = camera.para_tela(this->get_retanguloVisual());

    if (sprite == nullptr || !sprite->desenhar(renderizador, corpo))
    {
        SDL_SetRenderDrawColor(renderizador, COR_JOGADOR.r, COR_JOGADOR.g, COR_JOGADOR.b,
                               COR_JOGADOR.a);
        SDL_RenderFillRectF(renderizador, &corpo);

        SDL_SetRenderDrawColor(renderizador, COR_JOGADOR_BORDA.r, COR_JOGADOR_BORDA.g,
                               COR_JOGADOR_BORDA.b, COR_JOGADOR_BORDA.a);
        SDL_RenderDrawRectF(renderizador, &corpo);
    }

    const auto *arma = this->armas.frente();
    if (arma != nullptr)
    {
        const SDL_FPoint direcao = this->get_direcaoMira();
        const SDL_FPoint mao = camera.para_tela(this->get_centroX() + direcao.x * 12,
                                               this->get_centroY() + 6 + direcao.y * 12);
        IconeArma::desenhar_empunhada(renderizador, arma->get_tipo(), mao, this->anguloMira);
    }
    if (mostrarHitbox)
    {
        const SDL_FRect hitbox = camera.para_tela(this->get_retangulo());
        SDL_SetRenderDrawColor(renderizador, 100, 255, 140, 255);
        SDL_RenderDrawRectF(renderizador, &hitbox);
    }

}

void Jogador::rec_escudo(int quantidade)
{
    if (quantidade > 0)
    {
        this->escudo += std::min(quantidade, this->escudoMax - this->escudo);
    }
}

void Jogador::perder_escudo(int quantidade)
{
    if (quantidade > 0)
    {
        this->escudo -= std::min(quantidade, this->escudo);
    }
}

int Jogador::get_escudo() const
{
    return this->escudo;
}

int Jogador::get_escudoMax() const
{
    return this->escudoMax;
}

void Jogador::receber_dano(int quantidade)
{
    if (quantidade <= 0 || morto() || invulnerabilidade > 0) return;
    const int absorvido = std::min(escudo, quantidade);
    perder_escudo(absorvido);
    perder_vida(quantidade - absorvido);
    esperaEscudo = 3;
    cargaEscudo = 0;
    invulnerabilidade = 0.5f;
}
void Jogador::atualizar_defesa(float dt)
{
    invulnerabilidade = std::max(0.0f, invulnerabilidade - dt);
    const float recuperacao = std::max(0.0f, dt - esperaEscudo);
    esperaEscudo = std::max(0.0f, esperaEscudo - dt);
    if (morto() || escudo == escudoMax) { cargaEscudo = 0; return; }
    cargaEscudo += recuperacao * 10;
    const int pontos = static_cast<int>(cargaEscudo);
    rec_escudo(pontos);
    cargaEscudo -= pontos;
}
