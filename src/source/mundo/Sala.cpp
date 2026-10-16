#include "mundo/Sala.h"

#include <algorithm>
#include <cmath>
#include <random>
#include "interface/Texto.h"
#include <stdexcept>

#include "config/Config.h"
#include "graficos/IconeArma.h"

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
                               this->configuracao.dificuldade, configuracao.brutos, configuracao.boss);
        const TipoArma tipos[] = {TipoArma::SHOTGUN, TipoArma::LASER, TipoArma::DUPLA,
            TipoArma::RICOCHETE, TipoArma::ESPADA, TipoArma::ARCO, TipoArma::BOOMERANG};
        for (int i = 0; configuracao.armasIniciais && i < 7; ++i)
            armasChao.inserir({Armas(tipos[i]), 160.0f + (i % 2) * 130.0f,
                              configuracao.altura * (i / 2 + 1) / 5.0f, false});
        for (int i = 0; i < configuracao.baus; ++i)
            baus.inserir({configuracao.largura * (i + 1) / (configuracao.baus + 1),
                         configuracao.altura / 2, false});
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

void Sala::atualizar(float dt, Jogador &jogador)
{
    if (this->estado == EstadoSala::NAO_VISITADA)
    {
        return;
    }
    this->combate.atualizar(dt, jogador);
    static std::mt19937 sorteio(std::random_device{}());
    const TipoArma tipos[] = {TipoArma::PISTOLA, TipoArma::SHOTGUN, TipoArma::LASER,
        TipoArma::DUPLA, TipoArma::RICOCHETE, TipoArma::ESPADA, TipoArma::ARCO, TipoArma::BOOMERANG};
    for (auto &bau : baus)
    {
        if (!bau.aberto && std::hypot(jogador.get_centroX() - bau.x,
                                     jogador.get_centroY() - bau.y) < 64)
        {
            bau.aberto = true;
            for (int i = 0; i < 2; ++i)
                armasChao.inserir({Armas(tipos[std::uniform_int_distribution<int>(0, 7)(sorteio)]),
                                  bau.x + (i == 0 ? -40 : 40), bau.y + 48, false});
        }
    }
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
    for (int d = 0; d < 4; ++d)
    {
        const auto direcao = static_cast<Direcao>(d);
        if (configuracao.mapaRamificado ? configuracao.vizinhos[d] < 0 :
            (d > 1 || (d == 0 && !temAnterior))) continue;
        const bool aberta = porta_liberada(direcao);
        SDL_SetRenderDrawColor(renderizador, aberta ? 70 : 190, aberta ? 210 : 60, 85, 255);
        const auto porta = camera.para_tela(get_porta(direcao));
        SDL_RenderFillRectF(renderizador, &porta);
        if (configuracao.mapaRamificado)
        {
            const char *nomes[] = {"LOBBY", "A", "B", "C", "D", "E", "F"};
            Texto::desenhar_centralizado(renderizador, nomes[configuracao.vizinhos[d]],
                porta.x + porta.w / 2, porta.y + porta.h / 2 - 4, 1, {255,255,255,255});
        }
    }
    if (configuracao.boss && liberada())
    {
        auto centro = camera.para_tela(configuracao.largura / 2, configuracao.altura / 2);
        for (int r = 42; r > 0; --r)
        {
            SDL_SetRenderDrawColor(renderizador, 70 + r * 3, 50 + r * 3, 240, 255);
            for (int a = 0; a < 90; ++a)
            {
                float angulo = a * 6.2831853f / 90;
                SDL_RenderDrawPointF(renderizador, centro.x + std::cos(angulo) * r,
                                    centro.y + std::sin(angulo) * r * 0.65f);
            }
        }
        Texto::desenhar_centralizado(renderizador, "PROXIMA FASE", centro.x, centro.y + 38, 1, {220,240,255,255});
    }
    for (const auto &bau : baus)
    {
        const auto pos = camera.para_tela(bau.x, bau.y);
        auto bloco = [&](float x, float y, float w, float h, SDL_Color cor) {
            SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
            const SDL_FRect ret = {pos.x + x, pos.y + y, w, h};
            SDL_RenderFillRectF(renderizador, &ret);
        };
        bloco(-32, 18, 64, 8, {40,38,48,255});
        bloco(-30, -16, 60, 40, {48,27,30,255});
        bloco(-26, -12, 52, 32, {139,65,40,255});
        bloco(-24, 2, 48, 3, {78,35,30,255});
        bloco(-30, bau.aberto ? -30 : -20, 60, 18, {202,117,51,255});
        bloco(-26, bau.aberto ? -27 : -17, 52, 4, {246,183,84,255});
        if (bau.aberto) bloco(-23, -12, 46, 10, {28,23,34,255});
        bloco(-22, -16, 6, 38, {235,181,72,255});
        bloco(16, -16, 6, 38, {235,181,72,255});
        bloco(-7, -5, 14, 14, {255,218,107,255});
        bloco(-2, -1, 4, 7, {66,43,35,255});
    }
    for (const auto &item : armasChao)
    {
        if (item.coletada)
        {
            continue;
        }
        const auto pos = camera.para_tela(item.x, item.y);
        const SDL_FRect area = {pos.x - 18, pos.y - 18, 36, 36};
        IconeArma::desenhar(renderizador, item.arma.get_tipo(), area);
        Texto::desenhar_centralizado(renderizador, item.arma.get_nome(), pos.x, pos.y + 22,
                                    1, {255, 240, 180, 255});
    }
    this->combate.desenhar(renderizador, camera, mostrarHitboxes);
}

bool Sala::coletar_arma(Jogador &jogador)
{
    ArmaChao *maisProxima = nullptr;
    float distancia = 54;
    for (auto &item : armasChao)
    {
        const float atual = std::hypot(item.x - jogador.get_centroX(), item.y - jogador.get_centroY());
        if (!item.coletada && atual < distancia)
        {
            maisProxima = &item;
            distancia = atual;
        }
    }
    if (!maisProxima || !jogador.get_armas().inserir(maisProxima->arma))
    {
        return false;
    }
    maisProxima->coletada = true;
    return true;
}

void Sala::soltar_arma(Jogador &jogador)
{
    Armas arma;
    if (jogador.get_armas().retirar(arma))
        armasChao.inserir({arma, jogador.get_centroX(), jogador.get_centroY(), false});
}

void Sala::entrar(Jogador &jogador, Direcao entrada)
{
    entrar(jogador, entrada == Direcao::DIREITA);
    if (entrada == Direcao::CIMA || entrada == Direcao::BAIXO)
        jogador.set_posicao((configuracao.largura - jogador.get_largura()) / 2,
            entrada == Direcao::CIMA ? 90 : configuracao.altura - 90 - jogador.get_altura());
}
void Sala::liberar_boss(bool liberar) { bossLiberado = liberar; }
bool Sala::porta_liberada(Direcao direcao) const
{
    const int destino = configuracao.vizinhos[static_cast<int>(direcao)];
    return liberada() && (!configuracao.mapaRamificado ||
                          (destino >= 0 && (destino != 6 || bossLiberado)));
}
SDL_FRect Sala::get_porta(Direcao direcao) const
{
    if (direcao == Direcao::ESQUERDA || direcao == Direcao::DIREITA)
        return get_porta(direcao == Direcao::DIREITA);
    return {configuracao.largura / 2 - 48,
            direcao == Direcao::CIMA ? 0 : configuracao.altura - 18, 96, 18};
}
bool Sala::tocando_porta(const Jogador &jogador, Direcao direcao) const
{
    const auto porta = get_porta(direcao), corpo = jogador.get_retangulo();
    return SDL_HasIntersectionF(&porta, &corpo);
}
bool Sala::tocando_portal(const Jogador &jogador) const
{
    return configuracao.boss && liberada() &&
        std::hypot(jogador.get_centroX() - configuracao.largura / 2,
                   jogador.get_centroY() - configuracao.altura / 2) < 35;
}

void Sala::envelhecer_coletas(float dt) { combate.envelhecer_coracoes(dt); }
