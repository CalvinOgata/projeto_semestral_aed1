#include "mundo/Sala.h"

#include <algorithm>
#include <stdexcept>

#include "config/Config.h"

Sala::Sala(const ConfiguracaoSala &configuracao) : configuracao(configuracao)
{
    // Mantem espaco para entrada, portas e entidades sem clamp com limites invertidos.
    if (configuracao.largura < 400 || configuracao.altura < 300 ||
        configuracao.dificuldade < 1 || configuracao.quantidadeInimigos < 0)
    {
        throw std::invalid_argument("Configuracao de sala invalida");
    }
}

void Sala::entrar(Jogador &jogador, bool pelaDireita)
{
    if (this->estado == EstadoSala::NAO_VISITADA)
    {
        this->combate.reiniciar(this->get_limites(), this->configuracao.quantidadeInimigos,
                               this->configuracao.dificuldade);
        this->estado = EstadoSala::EM_COMBATE;
    }
    jogador.set_posicao(pelaDireita ? this->configuracao.largura - 90 - jogador.get_largura() : 90,
                       (this->configuracao.altura - jogador.get_altura()) / 2.0f);
    this->limitar_jogador(jogador);
    this->atualizar(0.0f, jogador);
}

void Sala::sair()
{
    // Tiros nao ficam suspensos para acertar o jogador/inimigos ao voltar.
    this->combate.limpar_projeteis();
}

void Sala::atualizar(float dt, const Jogador &jogador)
{
    if (this->estado == EstadoSala::NAO_VISITADA)
    {
        return;
    }
    this->combate.atualizar(dt, jogador);
    if (this->estado == EstadoSala::EM_COMBATE)
    {
        const bool eliminados = !this->configuracao.requisito.eliminarInimigos ||
                                this->combate.get_inimigosVivos() == 0;
        if (eliminados)
        {
            this->estado = EstadoSala::LIBERADA;
        }
    }
}

void Sala::disparar(Jogador &jogador)
{
    this->combate.disparar(jogador);
}

void Sala::limitar_jogador(Jogador &jogador) const
{
    jogador.set_posicao(std::clamp(jogador.get_x(), 0.0f,
                                  this->configuracao.largura - jogador.get_largura()),
                        std::clamp(jogador.get_y(), 0.0f,
                                  this->configuracao.altura - jogador.get_altura()));
}

SDL_FRect Sala::get_limites() const
{
    return {0, 0, this->configuracao.largura, this->configuracao.altura};
}

SDL_FRect Sala::get_porta(bool direita) const
{
    return {direita ? this->configuracao.largura - 18 : 0,
            this->configuracao.altura / 2.0f - 48, 18, 96};
}

bool Sala::tocando_porta(const Jogador &jogador, bool direita) const
{
    const SDL_FRect area = this->get_porta(direita);
    const SDL_FRect corpo = jogador.get_retangulo();
    return corpo.x <= area.x + area.w && corpo.x + corpo.w >= area.x &&
           corpo.y <= area.y + area.h && corpo.y + corpo.h >= area.y;
}

bool Sala::liberada() const
{
    return this->estado == EstadoSala::LIBERADA;
}

EstadoSala Sala::get_estado() const
{
    return this->estado;
}

int Sala::get_inimigosVivos() const
{
    return this->combate.get_inimigosVivos();
}

const ConfiguracaoSala &Sala::get_configuracao() const
{
    return this->configuracao;
}

void Sala::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool temAnterior,
                    bool mostrarHitboxes) const
{
    const SDL_FRect piso = camera.para_tela(this->get_limites());
    SDL_SetRenderDrawColor(renderizador, COR_MUNDO.r, COR_MUNDO.g, COR_MUNDO.b, 255);
    SDL_RenderFillRectF(renderizador, &piso);
    SDL_SetRenderDrawColor(renderizador, COR_GRADE.r, COR_GRADE.g, COR_GRADE.b, 255);
    for (float x = 0; x < this->configuracao.largura; x += PASSO_GRADE)
    {
        SDL_RenderDrawLineF(renderizador, piso.x + x, piso.y, piso.x + x, piso.y + piso.h);
    }
    for (float y = 0; y < this->configuracao.altura; y += PASSO_GRADE)
    {
        SDL_RenderDrawLineF(renderizador, piso.x, piso.y + y, piso.x + piso.w, piso.y + y);
    }
    // Paredes externas: a area caminhavel e o retangulo do piso.
    SDL_SetRenderDrawColor(renderizador, 40, 44, 54, 255);
    const SDL_FRect paredes[] = {{piso.x - 12, piso.y - 12, piso.w + 24, 12},
                                {piso.x - 12, piso.y + piso.h, piso.w + 24, 12},
                                {piso.x - 12, piso.y, 12, piso.h},
                                {piso.x + piso.w, piso.y, 12, piso.h}};
    for (const auto &parede : paredes)
    {
        SDL_RenderFillRectF(renderizador, &parede);
    }
    SDL_SetRenderDrawColor(renderizador, this->liberada() ? 70 : 190,
                           this->liberada() ? 210 : 60, 85, 255);
    const SDL_FRect direita = camera.para_tela(this->get_porta(true));
    SDL_RenderFillRectF(renderizador, &direita);
    if (temAnterior)
    {
        const SDL_FRect esquerda = camera.para_tela(this->get_porta(false));
        SDL_RenderFillRectF(renderizador, &esquerda);
    }
    this->combate.desenhar(renderizador, camera, mostrarHitboxes);
}
